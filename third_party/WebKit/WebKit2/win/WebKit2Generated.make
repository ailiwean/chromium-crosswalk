all:
    touch "$(WEBKITOUTPUTDIR)\buildfailed"
    -mkdir 2>NUL "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\cf\WKStringCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\cf\WKURLCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\cf\WKURLRequestCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\cf\WKURLResponseCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\win\WKBaseWin.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\win\WKCertificateInfoWin.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKArray.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKBase.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKCertificateInfo.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKData.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKDictionary.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKError.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKEvent.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKGeometry.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKMutableArray.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKMutableDictionary.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKNumber.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKSerializedScriptValue.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKSerializedScriptValuePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKString.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKStringPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKType.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKURL.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKURLRequest.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKURLResponse.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\Shared\API\c\WKUserContentURLPattern.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WebKit2.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBackForwardList.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBackForwardListItem.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKContext.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKContextPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFormSubmissionListener.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFrame.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFramePolicyListener.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKNativeEvent.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKNavigationData.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPage.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPageNamespace.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPreferences.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPreferencesPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\win\WKContextPrivateWin.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\win\WKView.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\win\WKViewPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\cpp\WKRetainPtr.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundle.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleBackForwardList.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleFrame.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleFramePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleHitTestResult.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleInitialize.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleNodeHandle.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleNodeHandlePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundlePage.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundlePagePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundlePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleRangeHandle.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleScriptWorld.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    bash build-generated-files.sh "$(WEBKITOUTPUTDIR)"
    -del "$(WEBKITOUTPUTDIR)\buildfailed"

clean:
    -del "$(WEBKITOUTPUTDIR)\buildfailed"
    -del /s /q "$(WEBKITOUTPUTDIR)\include\WebKit2"
