/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef BACKEND_EDITOR_H_INCLUDED
#define BACKEND_EDITOR_H_INCLUDED

#if HISE_IOS
#define SCROLLBAR_WIDTH 60
#else
#define SCROLLBAR_WIDTH 16
#endif


class ProcessorEditorPanel;
class PopupWindow;
class CustomKeyboard;
class BackendProcessor;
class ScriptContentContainer;


class BackendProcessorEditor: public AudioProcessorEditor,
							  public BackendCommandTarget,
							  public RestorableObject,
							  public GlobalScriptCompileListener,
                              public ComponentWithKeyboard,
                              public Label::Listener,
							  public ModalBaseWindow
{
public:

	BackendProcessorEditor(AudioProcessor *ownerProcessor, ValueTree &editorState);

	~BackendProcessorEditor();

	void showSettingsWindow();

	void showViewPanelPopup();

	void setRootProcessor(Processor *p, int scrollY=0);

	void rebuildContainer();

	void setRootProcessorWithUndo(Processor *p);

	void viewedComponentChanged()
	{
		owner->setScrollY(viewport->viewport->getViewPosition().getY());

		clearPopup();
		
		resized();
	};

	ValueTree exportAsValueTree() const override
	{
		ValueTree v("editorData");

		v.setProperty("width", getWidth(), nullptr);
		v.setProperty("height", getHeight(), nullptr);
		v.setProperty("keyboardShown", keyboard->isVisible(), nullptr);
		v.setProperty("macrosShown", macroKnobs->isVisible(), nullptr);
		v.setProperty("scrollPosition", viewport->viewport->getViewPosition().getY(), nullptr);
        v.setProperty("globalCodeFontSize", owner->getGlobalCodeFontSize(), nullptr);
		v.setProperty("autosaving", owner->getAutoSaver().isAutoSaving(), nullptr);

		MemoryBlock mb;

		

		mb.append(swatchColours, sizeof(Colour)*8);
		v.setProperty("swatchColours", mb.toBase64Encoding(), nullptr);

		v.addChild(referenceDebugArea->exportAsValueTree(), -1, nullptr);
		v.addChild(propertyDebugArea->exportAsValueTree(), -1, nullptr);

		return v;
	}

	void restoreFromValueTree(const ValueTree &v) override
	{
		if (v.getType() == Identifier("editorData"))
		{
			keyboard->setVisible(v.getProperty("keyboardShown"));
			macroKnobs->setVisible(v.getProperty("macrosShown"));

            setSize(v.getProperty("width", 900), v.getProperty("height", 700));
            
			owner->setScrollY(v.getProperty("scrollPosition", 0));
            
			const bool wasAutoSaving = v.getProperty("autosaving", true);

			if (wasAutoSaving) owner->getAutoSaver().enableAutoSaving();
			else owner->getAutoSaver().disableAutoSaving();

#if JUCE_WINDOWS
            
            owner->setGlobalCodeFontSize(v.getProperty("globalCodeFontSize", 14.0f));
#else
            owner->setGlobalCodeFontSize(v.getProperty("globalCodeFontSize", 13.0f));
#endif
            
			referenceDebugArea->restoreFromValueTree(v.getChildWithName(referenceDebugArea->getIdForArea()));
			propertyDebugArea->restoreFromValueTree(v.getChildWithName(propertyDebugArea->getIdForArea()));

			MemoryBlock mb;
			
			mb.fromBase64Encoding(v.getProperty("swatchColours").toString());

			if (mb.getSize() == sizeof(Colour) * 8)
			{
				storeSwatchColours((Colour*)(mb.getData()));
			}
		}
		else
		{
			setSize(900, 700);
		}
	}

	KeyboardFocusTraverser *createFocusTraverser() override { return new MidiKeyboardFocusTraverser(); };

	void paint(Graphics &g);

	void resized();

	void clearPopup();

	void refreshContainer(Processor *selectedProcessor)
	{
		if (container != nullptr)
		{
			const int y = viewport->viewport->getViewPositionY();

			setRootProcessor(container->getRootEditor()->getProcessor(), y);

			ProcessorEditor::Iterator iter(getRootContainer()->getRootEditor());

			while (ProcessorEditor *editor = iter.getNextEditor())
			{
				if (editor->getProcessor() == selectedProcessor)
				{
					editor->grabCopyAndPasteFocus();
				}
			}
		}
	}

	void scriptWasCompiled(JavascriptProcessor *sp) override;

	ModulatorSynthChain *getMainSynthChain() { return owner->synthChain; };

	const ModulatorSynthChain *getMainSynthChain() const { return owner->synthChain; };

	AutoPopupDebugComponent *getDebugComponent(bool lookInReferencePanel, int index)
	{
		if (lookInReferencePanel)
		{
			return dynamic_cast<AutoPopupDebugComponent*>(referenceDebugArea->getComponentForIndex(index));
		}
		else
		{
			return dynamic_cast<AutoPopupDebugComponent*>(propertyDebugArea->getComponentForIndex(index));
		}
	}

	ToolbarButton *getButtonForID(int id) const;

	void showPseudoModalWindow(Component *componentToShow, const String &title, bool ownComponent=false);

	void showModulatorTreePopup();
	
	void showProcessorPopup(Processor *p, Processor *parent);
	
    void labelTextChanged(Label *) override
    {
        owner->setChanged();
    }
    
    void clearPreset();

	void clearModuleList();

    

	void loadNewContainer(const File &f);

	void loadNewContainer(ValueTree &v);

	/** recursively scans all ProcessorEditors and adds them as submenu. 
	*
	*	@returns true if the Processor contains other Processors.
	*/
	bool addProcessorToPopupMenu(PopupMenu &m, Processor *p);

	void addProcessorToPanel(Processor *p);

	void removeProcessorFromPanel(Processor *p);

	/** This creates a path to the processor specified by the index from addProcessorEditorToPopup. */
	bool getIndexPath(Array<int> &path, Processor *p, const int searchIndex, int &counter);

	/** returns the ProcessorEditor for the path. */
	ProcessorEditor *getProcessorEditorFromPath(const Array<int> &path);

    void refreshInterfaceAfterPresetLoad();
    
	String createStringFromPath(const Array<int> &path);

	ProcessorEditorContainer *getRootContainer() { return container; };

	Component *getKeyboard() const override { return keyboard; }


	bool isFullScreenMode() const
	{
#if IS_STANDALONE_APP
        if(getParentComponent() == nullptr) return false;
        
        Component *kioskComponent = Desktop::getInstance().getKioskModeComponent();
        
        Component *parentparent = getParentComponent()->getParentComponent();
        
        return parentparent == kioskComponent;
#else
		return false;
#endif

	}
    


	BackendProcessor *getBackendProcessor() { return owner; };
	const BackendProcessor *getBackendProcessor() const { return owner; };
	void setToolBarPosition(int x, int y, int width, int height);
	void setViewportPositions(int viewportX, const int viewportY, const int viewportWidth, int viewportHeight);
	
	UndoManager * getViewUndoManager()
	{
		return owner->viewUndoManager;
	}

	SafeChangeBroadcaster &getModuleListNofifier() { return moduleListNotifier; }

	void rebuildModuleList(bool synchronous)
	{
		if (synchronous)
			moduleListNotifier.sendSynchronousChangeMessage();
		else
			moduleListNotifier.sendChangeMessage();
	}

	void setPluginPreviewWindow(PluginPreviewWindow *newWindow)
	{
		previewWindow = nullptr;

		previewWindow = newWindow;

		if (previewWindow != nullptr)
		{
#if HISE_IOS
            showPseudoModalWindow(newWindow, getMainSynthChain()->getId(), true);
#else
            
			newWindow->addToDesktop();
#endif
		}

	}

	bool isPluginPreviewShown() const;;
	bool isPluginPreviewCreatable() const;

	void storeSwatchColours(Colour *coloursFromColourPicker)
	{
		memcpy(swatchColours, coloursFromColourPicker, sizeof(Colour)*8);
	}

	void restoreSwatchColours(Colour *coloursFromColourPicker)
	{
		memcpy(coloursFromColourPicker, swatchColours, sizeof(Colour)*8);
	}

private:

	Colour swatchColours[8];

	friend class BackendProcessor;
	
	friend class BackendCommandTarget;
    
    StringArray menuNames;

	WeakReference<Processor> currentRootProcessor;

	LookAndFeel_V3 lookAndFeelV3;

	ScopedPointer<Toolbar> mainToolbar;
	ScopedPointer<ToolbarItemFactory> toolbarFactory;

	ScopedPointer<TooltipBar> tooltipBar;

	ScopedPointer<ProcessorEditorContainer> container;

	ScopedPointer<MacroComponent> macroKnobs;

    ScopedPointer<CachedViewport> viewport;
	ScopedPointer<CustomKeyboard> keyboard;
	ScopedPointer<MenuBarComponent> menuBar;

	SafeChangeBroadcaster moduleListNotifier;

	BackendProcessor *owner;

	ScopedPointer<CombinedDebugArea> referenceDebugArea;
	ScopedPointer<PropertyDebugArea> propertyDebugArea;

	ScopedPointer<ResizableBorderComponent> borderDragger;

	PopupLookAndFeel plaf;

	WeakReference<Component> currentPopupComponent;
	ScopedPointer<Component> ownedPopupComponent;

    WeakReference<CopyPasteTarget> currentlySelectedCopyableObject;
    
	ScopedPointer<ProcessorEditor> popupEditor;
	ScopedPointer<StupidRectangle> stupidRectangle;
	
	ScopedPointer<AudioDeviceDialog> currentDialog;

	ScopedPointer<AboutPage> aboutPage;

	ScopedPointer<ComponentBoundsConstrainer> constrainer;

	ScopedPointer<ProcessorList> list;

	ScopedPointer<VoiceCpuBpmComponent> cpuVoiceComponent;
	
	ScopedPointer<ShapeButton> backButton;
	ScopedPointer<ShapeButton> forwardButton;

	ScopedPointer<BreadcrumbComponent> breadCrumbComponent;

	ScopedPointer<PluginPreviewWindow> previewWindow;

	ScopedPointer<ThreadWithQuasiModalProgressWindow::Overlay> progressOverlay;

	bool rootEditorIsMainSynthChain;

    
#if HISE_IOS

	ScopedPointer<Component> menuRuler;

	ScopedPointer<ShapeButton> octaveUpButton;
	ScopedPointer<ShapeButton> octaveDownButton;

#endif

};

#endif
