// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "base/command_line.h"
#include "base/files/scoped_temp_dir.h"
#include "base/id_map.h"
#include "base/location.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/thread_task_runner_handle.h"
#include "components/dom_distiller/content/browser/distiller_javascript_utils.h"
#include "components/dom_distiller/content/browser/distiller_page_web_contents.h"
#include "components/dom_distiller/core/article_entry.h"
#include "components/dom_distiller/core/distilled_page_prefs.h"
#include "components/dom_distiller/core/distiller.h"
#include "components/dom_distiller/core/dom_distiller_service.h"
#include "components/dom_distiller/core/dom_distiller_store.h"
#include "components/dom_distiller/core/proto/distilled_article.pb.h"
#include "components/dom_distiller/core/proto/distilled_page.pb.h"
#include "components/dom_distiller/core/task_tracker.h"
#include "components/leveldb_proto/proto_database.h"
#include "components/leveldb_proto/proto_database_impl.h"
#include "components/pref_registry/testing_pref_service_syncable.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/isolated_world_ids.h"
#include "content/public/test/content_browser_test.h"
#include "content/shell/browser/shell.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "net/dns/mock_host_resolver.h"
#include "third_party/dom_distiller_js/dom_distiller.pb.h"
#include "ui/base/resource/resource_bundle.h"

using content::ContentBrowserTest;

namespace dom_distiller {

namespace {

typedef base::hash_map<std::string, std::string> FileToUrlMap;

}

// Factory for creating a Distiller that creates different DomDistillerOptions
// for different URLs, i.e. a specific kOriginalUrl option for each URL.
class TestDistillerFactoryImpl : public DistillerFactory {
 public:
  TestDistillerFactoryImpl(
      scoped_ptr<DistillerURLFetcherFactory> distiller_url_fetcher_factory,
      const dom_distiller::proto::DomDistillerOptions& dom_distiller_options,
      const FileToUrlMap& file_to_url_map)
      : distiller_url_fetcher_factory_(distiller_url_fetcher_factory.Pass()),
        dom_distiller_options_(dom_distiller_options),
        file_to_url_map_(file_to_url_map) {
  }

  ~TestDistillerFactoryImpl() override {}

  scoped_ptr<Distiller> CreateDistillerForUrl(const GURL& url) override {
    dom_distiller::proto::DomDistillerOptions options;
    options = dom_distiller_options_;
    FileToUrlMap::const_iterator it = file_to_url_map_.find(url.spec());
    if (it != file_to_url_map_.end()) {
      options.set_original_url(it->second);
    }
    scoped_ptr<DistillerImpl> distiller(new DistillerImpl(
        *distiller_url_fetcher_factory_, options));
    return distiller.Pass();
  }

 private:
  scoped_ptr<DistillerURLFetcherFactory> distiller_url_fetcher_factory_;
  dom_distiller::proto::DomDistillerOptions dom_distiller_options_;
  FileToUrlMap file_to_url_map_;
};

namespace {

// The url to distill.
const char* kUrlSwitch = "url";

// A space-separated list of urls to distill.
const char* kUrlsSwitch = "urls";

// Indicates that DNS resolution should be disabled for this test.
const char* kDisableDnsSwitch = "disable-dns";

// Will write the distilled output to the given file instead of to stdout.
const char* kOutputFile = "output-file";

// Indicates to output a serialized protocol buffer instead of human-readable
// output.
const char* kShouldOutputBinary = "output-binary";

// Indicates to output only the text of the article and not the enclosing html.
const char* kExtractTextOnly = "extract-text-only";

// Indicates to include debug output.
const char* kDebugLevel = "debug-level";

// The original URL of the page if |kUrlSwitch| is a file.
const char* kOriginalUrl = "original-url";

// A semi-colon-separated (i.e. ';') list of original URLs corresponding to
// "kUrlsSwitch".
const char* kOriginalUrls = "original-urls";

// Maximum number of concurrent started extractor requests.
const int kMaxExtractorTasks = 8;

scoped_ptr<DomDistillerService> CreateDomDistillerService(
    content::BrowserContext* context,
    const base::FilePath& db_path,
    const FileToUrlMap& file_to_url_map) {
  scoped_refptr<base::SequencedTaskRunner> background_task_runner =
      content::BrowserThread::GetBlockingPool()->GetSequencedTaskRunner(
          content::BrowserThread::GetBlockingPool()->GetSequenceToken());

  // TODO(cjhopman): use an in-memory database instead of an on-disk one with
  // temporary directory.
  scoped_ptr<leveldb_proto::ProtoDatabaseImpl<ArticleEntry> > db(
      new leveldb_proto::ProtoDatabaseImpl<ArticleEntry>(
          background_task_runner));
  scoped_ptr<DomDistillerStore> dom_distiller_store(
      new DomDistillerStore(db.Pass(), db_path));

  scoped_ptr<DistillerPageFactory> distiller_page_factory(
      new DistillerPageWebContentsFactory(context));
  scoped_ptr<DistillerURLFetcherFactory> distiller_url_fetcher_factory(
      new DistillerURLFetcherFactory(context->GetRequestContext()));

  dom_distiller::proto::DomDistillerOptions options;
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(kExtractTextOnly)) {
    options.set_extract_text_only(true);
  }
  int debug_level = 0;
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(kDebugLevel) &&
      base::StringToInt(
          base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
              kDebugLevel),
          &debug_level)) {
    options.set_debug_level(debug_level);
  }
  scoped_ptr<DistillerFactory> distiller_factory(
      new TestDistillerFactoryImpl(distiller_url_fetcher_factory.Pass(),
                                   options,
                                   file_to_url_map));

  // Setting up PrefService for DistilledPagePrefs.
  user_prefs::TestingPrefServiceSyncable* pref_service =
      new user_prefs::TestingPrefServiceSyncable();
  DistilledPagePrefs::RegisterProfilePrefs(pref_service->registry());

  return scoped_ptr<DomDistillerService>(new DomDistillerService(
      dom_distiller_store.Pass(),
      distiller_factory.Pass(),
      distiller_page_factory.Pass(),
      scoped_ptr<DistilledPagePrefs>(new DistilledPagePrefs(pref_service))));
}

void AddComponentsTestResources() {
  base::FilePath pak_file;
  base::FilePath pak_dir;
  PathService::Get(base::DIR_MODULE, &pak_dir);
  pak_file =
      pak_dir.Append(FILE_PATH_LITERAL("components_tests_resources.pak"));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_NONE);
}

bool WriteProtobufWithSize(
    const google::protobuf::MessageLite& message,
    google::protobuf::io::ZeroCopyOutputStream* output_stream) {
  google::protobuf::io::CodedOutputStream coded_output(output_stream);

  // Write the size.
  const int size = message.ByteSize();
  coded_output.WriteLittleEndian32(size);
  message.SerializeWithCachedSizes(&coded_output);
  return !coded_output.HadError();
}

std::string GetReadableArticleString(
    const DistilledArticleProto& article_proto) {
  std::stringstream output;
  output << "Article Title: " << article_proto.title() << std::endl;
  output << "# of pages: " << article_proto.pages_size() << std::endl;
  for (int i = 0; i < article_proto.pages_size(); ++i) {
    if (i > 0) output << std::endl;
    const DistilledPageProto& page = article_proto.pages(i);
    output << "Page " << i << std::endl;
    output << "URL: " << page.url() << std::endl;
    output << "Content: " << page.html() << std::endl;
    if (page.has_debug_info() && page.debug_info().has_log())
      output << "Log: " << page.debug_info().log() << std::endl;
    if (page.has_pagination_info()) {
      if (page.pagination_info().has_next_page()) {
        output << "Next Page: " << page.pagination_info().next_page()
               << std::endl;
      }
      if (page.pagination_info().has_prev_page()) {
        output << "Prev Page: " << page.pagination_info().prev_page()
               << std::endl;
      }
    }
  }
  return output.str();
}

}  // namespace

class ContentExtractionRequest : public ViewRequestDelegate {
 public:
  void Start(DomDistillerService* service, const gfx::Size& render_view_size,
             base::Closure finished_callback) {
    finished_callback_ = finished_callback;
    viewer_handle_ =
        service->ViewUrl(this,
                         service->CreateDefaultDistillerPage(render_view_size),
                         url_);
  }

  DistilledArticleProto GetArticleCopy() {
    return *article_proto_;
  }

  static ScopedVector<ContentExtractionRequest> CreateForCommandLine(
      const base::CommandLine& command_line,
      FileToUrlMap* file_to_url_map) {
    ScopedVector<ContentExtractionRequest> requests;
    if (command_line.HasSwitch(kUrlSwitch)) {
      GURL url;
      std::string url_string = command_line.GetSwitchValueASCII(kUrlSwitch);
      url = GURL(url_string);
      if (url.is_valid()) {
        requests.push_back(new ContentExtractionRequest(url));
        if (command_line.HasSwitch(kOriginalUrl)) {
          (*file_to_url_map)[url.spec()] =
              command_line.GetSwitchValueASCII(kOriginalUrl);
        }
      }
    } else if (command_line.HasSwitch(kUrlsSwitch)) {
      std::string urls_string = command_line.GetSwitchValueASCII(kUrlsSwitch);
      std::vector<std::string> urls = base::SplitString(
          urls_string, " ", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
      // Check for original-urls switch, which must exactly pair up with
      // |kUrlsSwitch| i.e. number of original urls must be same as that of
      // urls.
      std::vector<std::string> original_urls;
      if (command_line.HasSwitch(kOriginalUrls)) {
        std::string original_urls_string =
            command_line.GetSwitchValueASCII(kOriginalUrls);
        original_urls = base::SplitString(
            original_urls_string, " ",
            base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
        if (original_urls.size() != urls.size())
          original_urls.clear();
      }
      for (size_t i = 0; i < urls.size(); ++i) {
        GURL url(urls[i]);
        if (url.is_valid()) {
          requests.push_back(new ContentExtractionRequest(url));
          // Only regard non-empty original urls.
          if (!original_urls.empty() && !original_urls[i].empty()) {
              (*file_to_url_map)[url.spec()] = original_urls[i];
          }
        } else {
          ADD_FAILURE() << "Bad url";
        }
      }
    }
    if (requests.empty()) {
      ADD_FAILURE() << "No valid url provided";
    }

    return requests.Pass();
  }

 private:
  ContentExtractionRequest(const GURL& url) : url_(url) {}

  void OnArticleUpdated(ArticleDistillationUpdate article_update) override {}

  void OnArticleReady(const DistilledArticleProto* article_proto) override {
    article_proto_ = article_proto;
    CHECK(article_proto->pages_size()) << "Failed extracting " << url_;
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                  finished_callback_);
  }

  const DistilledArticleProto* article_proto_;
  scoped_ptr<ViewerHandle> viewer_handle_;
  GURL url_;
  base::Closure finished_callback_;
};

class ContentExtractor : public ContentBrowserTest {
 public:
  ContentExtractor()
      : pending_tasks_(0),
        max_tasks_(kMaxExtractorTasks),
        next_request_(0),
        output_data_(),
        protobuf_output_stream_(
            new google::protobuf::io::StringOutputStream(&output_data_)) {}

  // Change behavior of the default host resolver to avoid DNS lookup errors, so
  // we can make network calls.
  void SetUpOnMainThread() override {
    if (!base::CommandLine::ForCurrentProcess()->HasSwitch(kDisableDnsSwitch)) {
      EnableDNSLookupForThisTest();
    }
    CHECK(db_dir_.CreateUniqueTempDir());
    AddComponentsTestResources();
  }

  void TearDownOnMainThread() override { DisableDNSLookupForThisTest(); }

 protected:
  // Creates the DomDistillerService and creates and starts the extraction
  // request.
  void Start() {
    const base::CommandLine& command_line =
        *base::CommandLine::ForCurrentProcess();
    FileToUrlMap file_to_url_map;
    requests_ = ContentExtractionRequest::CreateForCommandLine(
        command_line, &file_to_url_map);
    content::BrowserContext* context =
        shell()->web_contents()->GetBrowserContext();
    service_ = CreateDomDistillerService(context,
                                         db_dir_.path(),
                                         file_to_url_map);
    PumpQueue();
  }

  void PumpQueue() {
    while (pending_tasks_ < max_tasks_ && next_request_ < requests_.size()) {
      requests_[next_request_]->Start(
          service_.get(),
          shell()->web_contents()->GetContainerBounds().size(),
          base::Bind(&ContentExtractor::FinishRequest, base::Unretained(this)));
      ++next_request_;
      ++pending_tasks_;
    }
  }

 private:
  // Change behavior of the default host resolver to allow DNS lookup
  // to proceed instead of being blocked by the test infrastructure.
  void EnableDNSLookupForThisTest() {
    // mock_host_resolver_override_ takes ownership of the resolver.
    scoped_refptr<net::RuleBasedHostResolverProc> resolver =
        new net::RuleBasedHostResolverProc(host_resolver());
    resolver->AllowDirectLookup("*");
    mock_host_resolver_override_.reset(
        new net::ScopedDefaultHostResolverProc(resolver.get()));
  }

  // We need to reset the DNS lookup when we finish, or the test will fail.
  void DisableDNSLookupForThisTest() {
    mock_host_resolver_override_.reset();
  }

  void FinishRequest() {
    --pending_tasks_;
    if (next_request_ == requests_.size() && pending_tasks_ == 0) {
      Finish();
    } else {
      PumpQueue();
    }
  }

  void DoArticleOutput() {
    const base::CommandLine& command_line =
        *base::CommandLine::ForCurrentProcess();
    for (size_t i = 0; i < requests_.size(); ++i) {
      const DistilledArticleProto& article = requests_[i]->GetArticleCopy();
      if (command_line.HasSwitch(kShouldOutputBinary)) {
        WriteProtobufWithSize(article, protobuf_output_stream_.get());
      } else {
        output_data_ += GetReadableArticleString(article) + "\n";
      }
    }

    if (command_line.HasSwitch(kOutputFile)) {
      base::FilePath filename = command_line.GetSwitchValuePath(kOutputFile);
      ASSERT_EQ(
          (int)output_data_.size(),
          base::WriteFile(filename, output_data_.c_str(), output_data_.size()));
    } else {
      VLOG(0) << output_data_;
    }
  }

  void Finish() {
    DoArticleOutput();
    requests_.clear();
    service_.reset();
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::MessageLoop::QuitWhenIdleClosure());
  }

  size_t pending_tasks_;
  size_t max_tasks_;
  size_t next_request_;

  base::ScopedTempDir db_dir_;
  scoped_ptr<net::ScopedDefaultHostResolverProc> mock_host_resolver_override_;
  scoped_ptr<DomDistillerService> service_;
  ScopedVector<ContentExtractionRequest> requests_;

  std::string output_data_;
  scoped_ptr<google::protobuf::io::StringOutputStream> protobuf_output_stream_;
};

IN_PROC_BROWSER_TEST_F(ContentExtractor, MANUAL_ExtractUrl) {
  SetDistillerJavaScriptWorldId(content::ISOLATED_WORLD_ID_CONTENT_END);
  Start();
  base::RunLoop().Run();
}

}  // namespace dom_distiller
