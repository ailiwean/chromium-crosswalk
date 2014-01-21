function blinkFilePaths()
{
    var paths = [
        "./Source/core/inspector",
        "./Source/core/inspector/BindingVisitors.z",
        "./Source/core/inspector/CodeGeneratorInspector.ty",
        "./Source/core/inspector/CodeGeneratorInspectorStrings.ty",
        "./Source/core/inspector/CodeGeneratorInspectorStrings.tyc",
        "./Source/core/inspector/CodeGeneratorInstrumentation.ty",
        "./Source/core/inspector/ConsoleAPITypes.z",
        "./Source/core/inspector/ConsoleMessage.bpp",
        "./Source/core/inspector/ConsoleMessage.z",
        "./Source/core/inspector/ContentSearchUtils.bpp",
        "./Source/core/inspector/ContentSearchUtils.z",
        "./Source/core/inspector/DOMEditor.bpp",
        "./Source/core/inspector/DOMEditor.z",
        "./Source/core/inspector/DOMPatchSupport.bpp",
        "./Source/core/inspector/DOMPatchSupport.z",
        "./Source/core/inspector/HeapGraphSerializer.bpp",
        "./Source/core/inspector/HeapGraphSerializer.z",
        "./Source/core/inspector/IdentifiersFactory.bpp",
        "./Source/core/inspector/IdentifiersFactory.z",
        "./Source/core/inspector/InjectedScript.bpp",
        "./Source/core/inspector/InjectedScript.z",
        "./Source/core/inspector/InjectedScriptBase.bpp",
        "./Source/core/inspector/InjectedScriptBase.z",
        "./Source/core/inspector/InjectedScriptCanvasModule.bpp",
        "./Source/core/inspector/InjectedScriptCanvasModule.z",
        "./Source/core/inspector/InjectedScriptCanvasModuleSource.pl",
        "./Source/core/inspector/InjectedScriptExterns.pl",
        "./Source/core/inspector/InjectedScriptHost.bpp",
        "./Source/core/inspector/InjectedScriptHost.z",
        "./Source/core/inspector/InjectedScriptHost.idl",
        "./Source/core/inspector/InjectedScriptManager.bpp",
        "./Source/core/inspector/InjectedScriptManager.z",
        "./Source/core/inspector/InjectedScriptModule.bpp",
        "./Source/core/inspector/InjectedScriptModule.z",
        "./Source/core/inspector/InjectedScriptSource.pl",
        "./Source/core/inspector/InspectorAgent.bpp",
        "./Source/core/inspector/InspectorAgent.z",
        "./Source/core/inspector/InspectorApplicationCacheAgent.bpp",
        "./Source/core/inspector/InspectorApplicationCacheAgent.z",
        "./Source/core/inspector/InspectorBaseAgent.bpp",
        "./Source/core/inspector/InspectorBaseAgent.z",
        "./Source/core/inspector/InspectorCSSAgent.bpp",
        "./Source/core/inspector/InspectorCSSAgent.z",
        "./Source/core/inspector/InspectorCanvasAgent.bpp",
        "./Source/core/inspector/InspectorCanvasAgent.z",
        "./Source/core/inspector/InspectorCanvasInstrumentation.z",
        "./Source/core/inspector/InspectorClient.bpp",
        "./Source/core/inspector/InspectorClient.z",
        "./Source/core/inspector/InspectorConsoleAgent.bpp",
        "./Source/core/inspector/InspectorConsoleAgent.z",
        "./Source/core/inspector/InspectorConsoleInstrumentation.z",
        "./Source/core/inspector/InspectorController.bpp",
        "./Source/core/inspector/InspectorController.z",
        "./Source/core/inspector/InspectorCounters.bpp",
        "./Source/core/inspector/InspectorCounters.z",
        "./Source/core/inspector/InspectorDOMAgent.bpp",
        "./Source/core/inspector/InspectorDOMAgent.z",
        "./Source/core/inspector/InspectorDOMDebuggerAgent.bpp",
        "./Source/core/inspector/InspectorDOMDebuggerAgent.z",
        "./Source/core/inspector/InspectorDOMStorageAgent.bpp",
        "./Source/core/inspector/InspectorDOMStorageAgent.z",
        "./Source/core/inspector/InspectorDatabaseAgent.bpp",
        "./Source/core/inspector/InspectorDatabaseAgent.z",
        "./Source/core/inspector/InspectorDatabaseInstrumentation.z",
        "./Source/core/inspector/InspectorDatabaseResource.bpp",
        "./Source/core/inspector/InspectorDatabaseResource.z",
        "./Source/core/inspector/InspectorDebuggerAgent.bpp",
        "./Source/core/inspector/InspectorDebuggerAgent.z",
        "./Source/core/inspector/InspectorFileSystemAgent.bpp",
        "./Source/core/inspector/InspectorFileSystemAgent.z",
        "./Source/core/inspector/InspectorFrontendChannel.z",
        "./Source/core/inspector/InspectorFrontendClient.z",
        "./Source/core/inspector/InspectorFrontendHost.bpp",
        "./Source/core/inspector/InspectorFrontendHost.z",
        "./Source/core/inspector/InspectorFrontendHost.idl",
        "./Source/core/inspector/InspectorHeapProfilerAgent.bpp",
        "./Source/core/inspector/InspectorHeapProfilerAgent.z",
        "./Source/core/inspector/InspectorHistory.bpp",
        "./Source/core/inspector/InspectorHistory.z",
        "./Source/core/inspector/InspectorIndexedDBAgent.bpp",
        "./Source/core/inspector/InspectorIndexedDBAgent.z",
        "./Source/core/inspector/InspectorInputAgent.bpp",
        "./Source/core/inspector/InspectorInputAgent.z",
        "./Source/core/inspector/InspectorInstrumentation.bpp",
        "./Source/core/inspector/InspectorInstrumentation.z",
        "./Source/core/inspector/InspectorInstrumentation.idl",
        "./Source/core/inspector/InspectorInstrumentationCustomInl.z",
        "./Source/core/inspector/InspectorLayerTreeAgent.bpp",
        "./Source/core/inspector/InspectorLayerTreeAgent.z",
        "./Source/core/inspector/InspectorMemoryAgent.bpp",
        "./Source/core/inspector/InspectorMemoryAgent.z",
        "./Source/core/inspector/InspectorOverlay.bpp",
        "./Source/core/inspector/InspectorOverlay.z",
        "./Source/core/inspector/InspectorOverlayHost.bpp",
        "./Source/core/inspector/InspectorOverlayHost.z",
        "./Source/core/inspector/InspectorOverlayHost.idl",
        "./Source/core/inspector/InspectorOverlayPage.ztml",
        "./Source/core/inspector/InspectorPageAgent.bpp",
        "./Source/core/inspector/InspectorPageAgent.z",
        "./Source/core/inspector/InspectorProfilerAgent.bpp",
        "./Source/core/inspector/InspectorProfilerAgent.z",
        "./Source/core/inspector/InspectorResourceAgent.bpp",
        "./Source/core/inspector/InspectorResourceAgent.z",
        "./Source/core/inspector/InspectorRuntimeAgent.bpp",
        "./Source/core/inspector/InspectorRuntimeAgent.z",
        "./Source/core/inspector/InspectorState.bpp",
        "./Source/core/inspector/InspectorState.z",
        "./Source/core/inspector/InspectorStateClient.z",
        "./Source/core/inspector/InspectorStyleSheet.bpp",
        "./Source/core/inspector/InspectorStyleSheet.z",
        "./Source/core/inspector/InspectorStyleTextEditor.bpp",
        "./Source/core/inspector/InspectorStyleTextEditor.z",
        "./Source/core/inspector/InspectorTimelineAgent.bpp",
        "./Source/core/inspector/InspectorTimelineAgent.z",
        "./Source/core/inspector/InspectorWorkerAgent.bpp",
        "./Source/core/inspector/InspectorWorkerAgent.z",
        "./Source/core/inspector/InspectorWorkerResource.z",
        "./Source/core/inspector/InstrumentingAgents.z",
        "./Source/core/inspector/JSONParser.bpp",
        "./Source/core/inspector/JSONParser.z",
        "./Source/core/inspector/JavaScriptCallFrame.bpp",
        "./Source/core/inspector/JavaScriptCallFrame.z",
        "./Source/core/inspector/JavaScriptCallFrame.idl",
        "./Source/core/inspector/MemoryInstrumentationImpl.bpp",
        "./Source/core/inspector/MemoryInstrumentationImpl.z",
        "./Source/core/inspector/NetworkResourcesData.bpp",
        "./Source/core/inspector/NetworkResourcesData.z",
        "./Source/core/inspector/PageConsoleAgent.bpp",
        "./Source/core/inspector/PageConsoleAgent.z",
        "./Source/core/inspector/PageDebuggerAgent.bpp",
        "./Source/core/inspector/PageDebuggerAgent.z",
        "./Source/core/inspector/PageRuntimeAgent.bpp",
        "./Source/core/inspector/PageRuntimeAgent.z",
        "./Source/core/inspector/ScriptArguments.bpp",
        "./Source/core/inspector/ScriptArguments.z",
        "./Source/core/inspector/ScriptBreakpoint.z",
        "./Source/core/inspector/ScriptCallFrame.bpp",
        "./Source/core/inspector/ScriptCallFrame.z",
        "./Source/core/inspector/ScriptCallStack.bpp",
        "./Source/core/inspector/ScriptCallStack.z",
        "./Source/core/inspector/ScriptDebugListener.z",
        "./Source/core/inspector/ScriptGCEventListener.z",
        "./Source/core/inspector/ScriptProfile.bpp",
        "./Source/core/inspector/ScriptProfile.z",
        "./Source/core/inspector/TimelineRecordFactory.bpp",
        "./Source/core/inspector/TimelineRecordFactory.z",
        "./Source/core/inspector/TimelineTraceEventProcessor.bpp",
        "./Source/core/inspector/TimelineTraceEventProcessor.z",
        "./Source/core/inspector/WorkerConsoleAgent.bpp",
        "./Source/core/inspector/WorkerConsoleAgent.z",
        "./Source/core/inspector/WorkerDebuggerAgent.bpp",
        "./Source/core/inspector/WorkerDebuggerAgent.z",
        "./Source/core/inspector/WorkerInspectorController.bpp",
        "./Source/core/inspector/WorkerInspectorController.z",
        "./Source/core/inspector/WorkerRuntimeAgent.bpp",
        "./Source/core/inspector/WorkerRuntimeAgent.z",
        "./Source/core/inspector/combine-javascript-resources.pl",
        "./Source/core/inspector/generate-inspector-protocol-version",
        "./Source/core/inspector/inline-javascript-imports.ty",
        "./Source/core/inspector/xxd.pl",
        "./Source/devtools",
        "./Source/devtools/Inspector-0.1.plon",
        "./Source/devtools/Inspector-1.0.plon",
        "./Source/devtools/OWNERS",
        "./Source/devtools/devtools.gyp",
        "./Source/devtools/front_end",
        "./Source/devtools/front_end/AdvancedSearchController.pl",
        "./Source/devtools/front_end/ApplicationCacheItemsView.pl",
        "./Source/devtools/front_end/ApplicationCacheModel.pl",
        "./Source/devtools/front_end/AuditCategories.pl",
        "./Source/devtools/front_end/AuditController.pl",
        "./Source/devtools/front_end/AuditFormatters.pl",
        "./Source/devtools/front_end/AuditLauncherView.pl",
        "./Source/devtools/front_end/AuditResultView.pl",
        "./Source/devtools/front_end/AuditRules.pl",
        "./Source/devtools/front_end/AuditsPanel.pl",
        "./Source/devtools/front_end/BottomUpProfileDataGridTree.pl",
        "./Source/devtools/front_end/BreakpointManager.pl",
        "./Source/devtools/front_end/BreakpointsSidebarPane.pl",
        "./Source/devtools/front_end/CPUProfileView.pl",
        "./Source/devtools/front_end/CSSMetadata.pl",
        "./Source/devtools/front_end/CSSNamedFlowCollectionsView.pl",
        "./Source/devtools/front_end/CSSNamedFlowView.pl",
        "./Source/devtools/front_end/CSSSelectorProfileView.pl",
        "./Source/devtools/front_end/CSSStyleModel.pl",
        "./Source/devtools/front_end/CSSStyleSheetMapping.pl",
        "./Source/devtools/front_end/CallStackSidebarPane.pl",
        "./Source/devtools/front_end/CanvasProfileView.pl",
        "./Source/devtools/front_end/Checkbox.pl",
        "./Source/devtools/front_end/CodeMirrorTextEditor.pl",
        "./Source/devtools/front_end/Color.pl",
        "./Source/devtools/front_end/CompilerScriptMapping.pl",
        "./Source/devtools/front_end/CompletionDictionary.pl",
        "./Source/devtools/front_end/ConsoleMessage.pl",
        "./Source/devtools/front_end/ConsoleModel.pl",
        "./Source/devtools/front_end/ConsolePanel.pl",
        "./Source/devtools/front_end/ConsoleView.pl",
        "./Source/devtools/front_end/ContentProvider.pl",
        "./Source/devtools/front_end/ContentProviderBasedProjectDelegate.pl",
        "./Source/devtools/front_end/ContentProviders.pl",
        "./Source/devtools/front_end/ContextMenu.pl",
        "./Source/devtools/front_end/CookieItemsView.pl",
        "./Source/devtools/front_end/CookieParser.pl",
        "./Source/devtools/front_end/CookiesTable.pl",
        "./Source/devtools/front_end/CountersGraph.pl",
        "./Source/devtools/front_end/DOMAgent.pl",
        "./Source/devtools/front_end/DOMBreakpointsSidebarPane.pl",
        "./Source/devtools/front_end/DOMExtension.pl",
        "./Source/devtools/front_end/DOMPresentationUtils.pl",
        "./Source/devtools/front_end/DOMStorage.pl",
        "./Source/devtools/front_end/DOMStorageItemsView.pl",
        "./Source/devtools/front_end/DOMSyntaxHighlighter.pl",
        "./Source/devtools/front_end/DataGrid.pl",
        "./Source/devtools/front_end/Database.pl",
        "./Source/devtools/front_end/DatabaseQueryView.pl",
        "./Source/devtools/front_end/DatabaseTableView.pl",
        "./Source/devtools/front_end/DebuggerModel.pl",
        "./Source/devtools/front_end/DebuggerScriptMapping.pl",
        "./Source/devtools/front_end/DefaultScriptMapping.pl",
        "./Source/devtools/front_end/DefaultTextEditor.pl",
        "./Source/devtools/front_end/DevToolsExtensionAPI.pl",
        "./Source/devtools/front_end/Dialog.pl",
        "./Source/devtools/front_end/DirectoryContentView.pl",
        "./Source/devtools/front_end/DockController.pl",
        "./Source/devtools/front_end/Drawer.pl",
        "./Source/devtools/front_end/ElementsPanel.pl",
        "./Source/devtools/front_end/ElementsPanelDescriptor.pl",
        "./Source/devtools/front_end/ElementsTreeOutline.pl",
        "./Source/devtools/front_end/EmptyView.pl",
        "./Source/devtools/front_end/EventListenersSidebarPane.pl",
        "./Source/devtools/front_end/ExtensionAPI.pl",
        "./Source/devtools/front_end/ExtensionAuditCategory.pl",
        "./Source/devtools/front_end/ExtensionPanel.pl",
        "./Source/devtools/front_end/ExtensionRegistryStub.pl",
        "./Source/devtools/front_end/ExtensionServer.pl",
        "./Source/devtools/front_end/ExtensionView.pl",
        "./Source/devtools/front_end/FileContentView.pl",
        "./Source/devtools/front_end/FileManager.pl",
        "./Source/devtools/front_end/FilePathScoreFunction.pl",
        "./Source/devtools/front_end/FileSystemMapping.pl",
        "./Source/devtools/front_end/FileSystemModel.pl",
        "./Source/devtools/front_end/FileSystemProjectDelegate.pl",
        "./Source/devtools/front_end/FileSystemView.pl",
        "./Source/devtools/front_end/FileUtils.pl",
        "./Source/devtools/front_end/FilteredItemSelectionDialog.pl",
        "./Source/devtools/front_end/FilteredItemSelectionDialog.pl~",
        "./Source/devtools/front_end/FlameChart.pl",
        "./Source/devtools/front_end/FontView.pl",
        "./Source/devtools/front_end/GoToLineDialog.pl",
        "./Source/devtools/front_end/HAREntry.pl",
        "./Source/devtools/front_end/HandlerRegistry.pl",
        "./Source/devtools/front_end/HeapSnapshot.pl",
        "./Source/devtools/front_end/HeapSnapshotDataGrids.pl",
        "./Source/devtools/front_end/HeapSnapshotGridNodes.pl",
        "./Source/devtools/front_end/HeapSnapshotLoader.pl",
        "./Source/devtools/front_end/HeapSnapshotProxy.pl",
        "./Source/devtools/front_end/HeapSnapshotView.pl",
        "./Source/devtools/front_end/HeapSnapshotWorker.pl",
        "./Source/devtools/front_end/HeapSnapshotWorkerDispatcher.pl",
        "./Source/devtools/front_end/HelpScreen.pl",
        "./Source/devtools/front_end/ImageView.pl",
        "./Source/devtools/front_end/Images",
        "./Source/devtools/front_end/Images/addIcon.png",
        "./Source/devtools/front_end/Images/applicationCache.png",
        "./Source/devtools/front_end/Images/back.png",
        "./Source/devtools/front_end/Images/breakpoint2.png",
        "./Source/devtools/front_end/Images/breakpoint2_2x.png",
        "./Source/devtools/front_end/Images/breakpointBorder.png",
        "./Source/devtools/front_end/Images/breakpointConditional2.png",
        "./Source/devtools/front_end/Images/breakpointConditional2_2x.png",
        "./Source/devtools/front_end/Images/breakpointConditionalBorder.png",
        "./Source/devtools/front_end/Images/breakpointConditionalCounterBorder.png",
        "./Source/devtools/front_end/Images/breakpointCounterBorder.png",
        "./Source/devtools/front_end/Images/checker.png",
        "./Source/devtools/front_end/Images/cookie.png",
        "./Source/devtools/front_end/Images/database.png",
        "./Source/devtools/front_end/Images/databaseTable.png",
        "./Source/devtools/front_end/Images/deleteIcon.png",
        "./Source/devtools/front_end/Images/domain.png",
        "./Source/devtools/front_end/Images/fileSystem.png",
        "./Source/devtools/front_end/Images/forward.png",
        "./Source/devtools/front_end/Images/frame.png",
        "./Source/devtools/front_end/Images/glossyHeader.png",
        "./Source/devtools/front_end/Images/glossyHeaderPressed.png",
        "./Source/devtools/front_end/Images/glossyHeaderSelected.png",
        "./Source/devtools/front_end/Images/glossyHeaderSelectedPressed.png",
        "./Source/devtools/front_end/Images/graphLabelCalloutLeft.png",
        "./Source/devtools/front_end/Images/graphLabelCalloutRight.png",
        "./Source/devtools/front_end/Images/indexedDB.png",
        "./Source/devtools/front_end/Images/indexedDBIndex.png",
        "./Source/devtools/front_end/Images/indexedDBObjectStore.png",
        "./Source/devtools/front_end/Images/localStorage.png",
        "./Source/devtools/front_end/Images/namedFlowOverflow.png",
        "./Source/devtools/front_end/Images/paneAddButtons.png",
        "./Source/devtools/front_end/Images/paneElementStateButtons.png",
        "./Source/devtools/front_end/Images/paneFilterButtons.png",
        "./Source/devtools/front_end/Images/paneRefreshButtons.png",
        "./Source/devtools/front_end/Images/paneSettingsButtons.png",
        "./Source/devtools/front_end/Images/popoverArrows.png",
        "./Source/devtools/front_end/Images/popoverBackground.png",
        "./Source/devtools/front_end/Images/profileGroupIcon.png",
        "./Source/devtools/front_end/Images/profileIcon.png",
        "./Source/devtools/front_end/Images/profileSmallIcon.png",
        "./Source/devtools/front_end/Images/programCounterBorder.png",
        "./Source/devtools/front_end/Images/radioDot.png",
        "./Source/devtools/front_end/Images/regionEmpty.png",
        "./Source/devtools/front_end/Images/regionFit.png",
        "./Source/devtools/front_end/Images/regionOverset.png",
        "./Source/devtools/front_end/Images/resourceCSSIcon.png",
        "./Source/devtools/front_end/Images/resourceDocumentIcon.png",
        "./Source/devtools/front_end/Images/resourceDocumentIconSmall.png",
        "./Source/devtools/front_end/Images/resourceJSIcon.png",
        "./Source/devtools/front_end/Images/resourcePlainIcon.png",
        "./Source/devtools/front_end/Images/resourcePlainIconSmall.png",
        "./Source/devtools/front_end/Images/resourcesTimeGraphIcon.png",
        "./Source/devtools/front_end/Images/searchNext.png",
        "./Source/devtools/front_end/Images/searchPrev.png",
        "./Source/devtools/front_end/Images/searchSmallBlue.png",
        "./Source/devtools/front_end/Images/searchSmallBrightBlue.png",
        "./Source/devtools/front_end/Images/searchSmallGray.png",
        "./Source/devtools/front_end/Images/searchSmallWhite.png",
        "./Source/devtools/front_end/Images/segment.png",
        "./Source/devtools/front_end/Images/segmentEnd.png",
        "./Source/devtools/front_end/Images/segmentHover.png",
        "./Source/devtools/front_end/Images/segmentHoverEnd.png",
        "./Source/devtools/front_end/Images/segmentSelected.png",
        "./Source/devtools/front_end/Images/segmentSelectedEnd.png",
        "./Source/devtools/front_end/Images/sessionStorage.png",
        "./Source/devtools/front_end/Images/settingsListRemove.png",
        "./Source/devtools/front_end/Images/settingsListRemove_2x.png",
        "./Source/devtools/front_end/Images/spinner.gif",
        "./Source/devtools/front_end/Images/spinnerActive.gif",
        "./Source/devtools/front_end/Images/spinnerActiveSelected.gif",
        "./Source/devtools/front_end/Images/spinnerInactive.gif",
        "./Source/devtools/front_end/Images/spinnerInactiveSelected.gif",
        "./Source/devtools/front_end/Images/src",
        "./Source/devtools/front_end/Images/src/breakpoints2.svg",
        "./Source/devtools/front_end/Images/src/settingListRemove.svg",
        "./Source/devtools/front_end/Images/src/statusbarButtonGlyphs.svg",
        "./Source/devtools/front_end/Images/statusbarButtonGlyphs.png",
        "./Source/devtools/front_end/Images/statusbarButtonGlyphs2x.png",
        "./Source/devtools/front_end/Images/statusbarResizerHorizontal.png",
        "./Source/devtools/front_end/Images/statusbarResizerVertical.png",
        "./Source/devtools/front_end/Images/thumbActiveHoriz.png",
        "./Source/devtools/front_end/Images/thumbActiveVert.png",
        "./Source/devtools/front_end/Images/thumbHoriz.png",
        "./Source/devtools/front_end/Images/thumbHoverHoriz.png",
        "./Source/devtools/front_end/Images/thumbHoverVert.png",
        "./Source/devtools/front_end/Images/thumbVert.png",
        "./Source/devtools/front_end/Images/timelineHollowPillBlue.png",
        "./Source/devtools/front_end/Images/timelineHollowPillGray.png",
        "./Source/devtools/front_end/Images/timelineHollowPillGreen.png",
        "./Source/devtools/front_end/Images/timelineHollowPillOrange.png",
        "./Source/devtools/front_end/Images/timelineHollowPillPurple.png",
        "./Source/devtools/front_end/Images/timelineHollowPillRed.png",
        "./Source/devtools/front_end/Images/timelineHollowPillYellow.png",
        "./Source/devtools/front_end/Images/timelinePillBlue.png",
        "./Source/devtools/front_end/Images/timelinePillGray.png",
        "./Source/devtools/front_end/Images/timelinePillGreen.png",
        "./Source/devtools/front_end/Images/timelinePillOrange.png",
        "./Source/devtools/front_end/Images/timelinePillPurple.png",
        "./Source/devtools/front_end/Images/timelinePillRed.png",
        "./Source/devtools/front_end/Images/timelinePillYellow.png",
        "./Source/devtools/front_end/Images/toolbarIcons.png",
        "./Source/devtools/front_end/Images/toolbarIconsSmall.png",
        "./Source/devtools/front_end/Images/toolbarItemSelected.png",
        "./Source/devtools/front_end/Images/trackHoriz.png",
        "./Source/devtools/front_end/Images/trackVert.png",
        "./Source/devtools/front_end/IndexedDBModel.pl",
        "./Source/devtools/front_end/IndexedDBViews.pl",
        "./Source/devtools/front_end/InspectElementModeController.pl",
        "./Source/devtools/front_end/InspectorBackend.pl",
        "./Source/devtools/front_end/InspectorFrontendAPI.pl",
        "./Source/devtools/front_end/InspectorFrontendHostStub.pl",
        "./Source/devtools/front_end/InspectorView.pl",
        "./Source/devtools/front_end/IsolatedFileSystem.pl",
        "./Source/devtools/front_end/IsolatedFileSystemManager.pl",
        "./Source/devtools/front_end/JSHeapSnapshot.pl",
        "./Source/devtools/front_end/JavaScriptFormatter.pl",
        "./Source/devtools/front_end/JavaScriptSourceFrame.pl",
        "./Source/devtools/front_end/KeyboardShortcut.pl",
        "./Source/devtools/front_end/Linkifier.pl",
        "./Source/devtools/front_end/LiveEditSupport.pl",
        "./Source/devtools/front_end/MemoryStatistics.pl",
        "./Source/devtools/front_end/MetricsSidebarPane.pl",
        "./Source/devtools/front_end/NativeBreakpointsSidebarPane.pl",
        "./Source/devtools/front_end/NativeHeapSnapshot.pl",
        "./Source/devtools/front_end/NativeMemoryGraph.pl",
        "./Source/devtools/front_end/NativeMemorySnapshotView.pl",
        "./Source/devtools/front_end/NavigatorOverlayController.pl",
        "./Source/devtools/front_end/NavigatorView.pl",
        "./Source/devtools/front_end/NetworkItemView.pl",
        "./Source/devtools/front_end/NetworkLog.pl",
        "./Source/devtools/front_end/NetworkManager.pl",
        "./Source/devtools/front_end/NetworkPanel.pl",
        "./Source/devtools/front_end/NetworkPanelDescriptor.pl",
        "./Source/devtools/front_end/NetworkRequest.pl",
        "./Source/devtools/front_end/NetworkUISourceCodeProvider.pl",
        "./Source/devtools/front_end/OWNERS",
        "./Source/devtools/front_end/Object.pl",
        "./Source/devtools/front_end/ObjectPopoverHelper.pl",
        "./Source/devtools/front_end/ObjectPropertiesSection.pl",
        "./Source/devtools/front_end/OverridesView.pl",
        "./Source/devtools/front_end/OverviewGrid.pl",
        "./Source/devtools/front_end/Panel.pl",
        "./Source/devtools/front_end/ParsedURL.pl",
        "./Source/devtools/front_end/Placard.pl",
        "./Source/devtools/front_end/Popover.pl",
        "./Source/devtools/front_end/PresentationConsoleMessageHelper.pl",
        "./Source/devtools/front_end/ProfileDataGridTree.pl",
        "./Source/devtools/front_end/ProfileLauncherView.pl",
        "./Source/devtools/front_end/ProfilesPanel.pl",
        "./Source/devtools/front_end/ProfilesPanelDescriptor.pl",
        "./Source/devtools/front_end/Progress.pl",
        "./Source/devtools/front_end/ProgressIndicator.pl",
        "./Source/devtools/front_end/PropertiesSection.pl",
        "./Source/devtools/front_end/PropertiesSidebarPane.pl",
        "./Source/devtools/front_end/RawSourceCode.pl",
        "./Source/devtools/front_end/RemoteObject.pl",
        "./Source/devtools/front_end/RequestCookiesView.pl",
        "./Source/devtools/front_end/RequestHTMLView.pl",
        "./Source/devtools/front_end/RequestHeadersView.pl",
        "./Source/devtools/front_end/RequestJSONView.pl",
        "./Source/devtools/front_end/RequestPreviewView.pl",
        "./Source/devtools/front_end/RequestResponseView.pl",
        "./Source/devtools/front_end/RequestTimingView.pl",
        "./Source/devtools/front_end/RequestView.pl",
        "./Source/devtools/front_end/Resource.pl",
        "./Source/devtools/front_end/ResourceScriptMapping.pl",
        "./Source/devtools/front_end/ResourceTreeModel.pl",
        "./Source/devtools/front_end/ResourceType.pl",
        "./Source/devtools/front_end/ResourceUtils.pl",
        "./Source/devtools/front_end/ResourceView.pl",
        "./Source/devtools/front_end/ResourceWebSocketFrameView.pl",
        "./Source/devtools/front_end/ResourcesPanel.pl",
        "./Source/devtools/front_end/RevisionHistoryView.pl",
        "./Source/devtools/front_end/RuntimeModel.pl",
        "./Source/devtools/front_end/SASSSourceMapping.pl",
        "./Source/devtools/front_end/ScopeChainSidebarPane.pl",
        "./Source/devtools/front_end/Script.pl",
        "./Source/devtools/front_end/ScriptFormatter.pl",
        "./Source/devtools/front_end/ScriptFormatterWorker.pl",
        "./Source/devtools/front_end/ScriptSnippetModel.pl",
        "./Source/devtools/front_end/ScriptsNavigator.pl",
        "./Source/devtools/front_end/ScriptsPanel.pl",
        "./Source/devtools/front_end/ScriptsPanelDescriptor.pl",
        "./Source/devtools/front_end/ScriptsSearchScope.pl",
        "./Source/devtools/front_end/SearchController.pl",
        "./Source/devtools/front_end/Section.pl",
        "./Source/devtools/front_end/Settings.pl",
        "./Source/devtools/front_end/SettingsScreen.pl",
        "./Source/devtools/front_end/ShortcutsScreen.pl",
        "./Source/devtools/front_end/ShowMoreDataGridNode.pl",
        "./Source/devtools/front_end/SidebarOverlay.pl",
        "./Source/devtools/front_end/SidebarPane.pl",
        "./Source/devtools/front_end/SidebarTreeElement.pl",
        "./Source/devtools/front_end/SidebarView.pl",
        "./Source/devtools/front_end/SimpleWorkspaceProvider.pl",
        "./Source/devtools/front_end/SnippetStorage.pl",
        "./Source/devtools/front_end/SoftContextMenu.pl",
        "./Source/devtools/front_end/SourceCSSTokenizer.pl",
        "./Source/devtools/front_end/SourceCSSTokenizer.re2js",
        "./Source/devtools/front_end/SourceFrame.pl",
        "./Source/devtools/front_end/SourceHTMLTokenizer.pl",
        "./Source/devtools/front_end/SourceHTMLTokenizer.re2js",
        "./Source/devtools/front_end/SourceJavaScriptTokenizer.pl",
        "./Source/devtools/front_end/SourceJavaScriptTokenizer.re2js",
        "./Source/devtools/front_end/SourceMap.pl",
        "./Source/devtools/front_end/SourceMapping.pl",
        "./Source/devtools/front_end/SourceTokenizer.pl",
        "./Source/devtools/front_end/Spectrum.pl",
        "./Source/devtools/front_end/SplitView.pl",
        "./Source/devtools/front_end/StatusBarButton.pl",
        "./Source/devtools/front_end/StyleSheetOutlineDialog.pl",
        "./Source/devtools/front_end/StylesSidebarPane.pl",
        "./Source/devtools/front_end/StylesSourceMapping.pl",
        "./Source/devtools/front_end/SuggestBox.pl",
        "./Source/devtools/front_end/TabbedEditorContainer.pl",
        "./Source/devtools/front_end/TabbedPane.pl",
        "./Source/devtools/front_end/TestController.pl",
        "./Source/devtools/front_end/Tests.pl",
        "./Source/devtools/front_end/TextEditor.pl",
        "./Source/devtools/front_end/TextEditorHighlighter.pl",
        "./Source/devtools/front_end/TextEditorModel.pl",
        "./Source/devtools/front_end/TextPrompt.pl",
        "./Source/devtools/front_end/TextUtils.pl",
        "./Source/devtools/front_end/TimelineFrameController.pl",
        "./Source/devtools/front_end/TimelineGrid.pl",
        "./Source/devtools/front_end/TimelineManager.pl",
        "./Source/devtools/front_end/TimelineModel.pl",
        "./Source/devtools/front_end/TimelineOverviewPane.pl",
        "./Source/devtools/front_end/TimelinePanel.pl",
        "./Source/devtools/front_end/TimelinePanelDescriptor.pl",
        "./Source/devtools/front_end/TimelinePresentationModel.pl",
        "./Source/devtools/front_end/Toolbar.pl",
        "./Source/devtools/front_end/TopDownProfileDataGridTree.pl",
        "./Source/devtools/front_end/UISourceCode.pl",
        "./Source/devtools/front_end/UISourceCodeFrame.pl",
        "./Source/devtools/front_end/UIString.pl",
        "./Source/devtools/front_end/UIUtils.pl",
        "./Source/devtools/front_end/UglifyJS",
        "./Source/devtools/front_end/UglifyJS/parse-js.pl",
        "./Source/devtools/front_end/UserAgentSupport.pl",
        "./Source/devtools/front_end/UserMetrics.pl",
        "./Source/devtools/front_end/View.pl",
        "./Source/devtools/front_end/ViewportControl.pl",
        "./Source/devtools/front_end/WatchExpressionsSidebarPane.pl",
        "./Source/devtools/front_end/WorkerManager.pl",
        "./Source/devtools/front_end/WorkersSidebarPane.pl",
        "./Source/devtools/front_end/Workspace.pl",
        "./Source/devtools/front_end/auditsPanel.mss",
        "./Source/devtools/front_end/breadcrumbList.mss",
        "./Source/devtools/front_end/breakpointsList.mss",
        "./Source/devtools/front_end/buildSystemOnly.pl",
        "./Source/devtools/front_end/canvasProfiler.mss",
        "./Source/devtools/front_end/cm",
        "./Source/devtools/front_end/cm/LICENSE",
        "./Source/devtools/front_end/cm/closebrackets.pl",
        "./Source/devtools/front_end/cm/cmdevtools.mss",
        "./Source/devtools/front_end/cm/codemirror.mss",
        "./Source/devtools/front_end/cm/codemirror.pl",
        "./Source/devtools/front_end/cm/comment.pl",
        "./Source/devtools/front_end/cm/css.pl",
        "./Source/devtools/front_end/cm/htmlmixed.pl",
        "./Source/devtools/front_end/cm/javascript.pl",
        "./Source/devtools/front_end/cm/markselection.pl",
        "./Source/devtools/front_end/cm/matchbrackets.pl",
        "./Source/devtools/front_end/cm/overlay.pl",
        "./Source/devtools/front_end/cm/showhint.mss",
        "./Source/devtools/front_end/cm/showhint.pl",
        "./Source/devtools/front_end/cm/xml.pl",
        "./Source/devtools/front_end/cssNamedFlows.mss",
        "./Source/devtools/front_end/dataGrid.mss",
        "./Source/devtools/front_end/dialog.mss",
        "./Source/devtools/front_end/elementsPanel.mss",
        "./Source/devtools/front_end/externs.pl",
        "./Source/devtools/front_end/filteredItemSelectionDialog.mss",
        "./Source/devtools/front_end/flameChart.mss",
        "./Source/devtools/front_end/heapProfiler.mss",
        "./Source/devtools/front_end/helpScreen.mss",
        "./Source/devtools/front_end/indexedDBViews.mss",
        "./Source/devtools/front_end/inspector.mss",
        "./Source/devtools/front_end/inspector.ztml",
        "./Source/devtools/front_end/inspector.pl",
        "./Source/devtools/front_end/inspectorCommon.mss",
        "./Source/devtools/front_end/inspectorSyntaxHighlight.mss",
        "./Source/devtools/front_end/jsdifflib.pl",
        "./Source/devtools/front_end/nativeMemoryProfiler.mss",
        "./Source/devtools/front_end/navigatorView.mss",
        "./Source/devtools/front_end/networkLogView.mss",
        "./Source/devtools/front_end/networkPanel.mss",
        "./Source/devtools/front_end/panelEnablerView.mss",
        "./Source/devtools/front_end/popover.mss",
        "./Source/devtools/front_end/profilesPanel.mss",
        "./Source/devtools/front_end/protocol_externs.pl",
        "./Source/devtools/front_end/resourceView.mss",
        "./Source/devtools/front_end/resourcesPanel.mss",
        "./Source/devtools/front_end/revisionHistory.mss",
        "./Source/devtools/front_end/scriptsPanel.mss",
        "./Source/devtools/front_end/sidebarPane.mss",
        "./Source/devtools/front_end/spectrum.mss",
        "./Source/devtools/front_end/splitView.mss",
        "./Source/devtools/front_end/tabbedPane.mss",
        "./Source/devtools/front_end/test-runner.ztml",
        "./Source/devtools/front_end/textEditor.mss",
        "./Source/devtools/front_end/textPrompt.mss",
        "./Source/devtools/front_end/timelinePanel.mss",
        "./Source/devtools/front_end/treeoutline.pl",
        "./Source/devtools/front_end/utilities.pl",
        "./Source/devtools/protocol.plon",
        "./Source/devtools/scripts",
        "./Source/devtools/scripts/CodeGeneratorFrontend.ty",
        "./Source/devtools/scripts/check_injected_webgl_calls_info.ty",
        "./Source/devtools/scripts/closure",
        "./Source/devtools/scripts/closure/COPYING",
        "./Source/devtools/scripts/closure/README",
        "./Source/devtools/scripts/closure/compiler.jar",
        "./Source/devtools/scripts/compile_frontend.ty",
        "./Source/devtools/scripts/concatenate_css_files.ty",
        "./Source/devtools/scripts/concatenate_js_files.ty",
        "./Source/devtools/scripts/generate_devtools_extension_api.ty",
        "./Source/devtools/scripts/generate_devtools_grd.ty",
        "./Source/devtools/scripts/generate_devtools_html.ty",
        "./Source/devtools/scripts/generate_protocol_externs.ty",
        "./Source/devtools/scripts/generate_protocol_externs.tyc",
        "./Source/devtools/scripts/inline_js_imports.ty",
        "./Source/devtools/scripts/jsmin.ty",
        "./Source/devtools/scripts/jsmin.tyc"
    ];
    return paths.join(":");
}
