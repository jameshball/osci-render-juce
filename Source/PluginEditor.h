#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EffectsComponent.h"
#include "MainComponent.h"
#include "LuaComponent.h"
#include "ObjComponent.h"
#include "components/VolumeComponent.h"
#include "components/MainMenuBarModel.h"
#include "LookAndFeel.h"


class OscirenderAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::CodeDocument::Listener, public juce::AsyncUpdater, public juce::ChangeListener {
public:
    OscirenderAudioProcessorEditor(OscirenderAudioProcessor&);
    ~OscirenderAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void initialiseCodeEditors();
    void addCodeEditor(int index);
    void removeCodeEditor(int index);
    void fileUpdated(juce::String fileName);
    void handleAsyncUpdate() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void editPerspectiveFunction(bool enabled);

    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void updateTitle();

    std::atomic<bool> editingPerspective = false;

    OscirenderLookAndFeel lookAndFeel;
private:
    OscirenderAudioProcessor& audioProcessor;
    
    MainComponent main{audioProcessor, *this};
    LuaComponent lua{audioProcessor, *this};
    ObjComponent obj{audioProcessor, *this};
    EffectsComponent effects{audioProcessor, *this};
    VolumeComponent volume{audioProcessor};
    std::vector<std::shared_ptr<juce::CodeDocument>> codeDocuments;
    std::vector<std::shared_ptr<juce::CodeEditorComponent>> codeEditors;
    juce::LuaTokeniser luaTokeniser;
    juce::XmlTokeniser xmlTokeniser;
	juce::ShapeButton collapseButton;
    std::shared_ptr<juce::CodeDocument> perspectiveCodeDocument = std::make_shared<juce::CodeDocument>();
    std::shared_ptr<juce::CodeEditorComponent> perspectiveCodeEditor = std::make_shared<juce::CodeEditorComponent>(*perspectiveCodeDocument, &luaTokeniser);

    std::unique_ptr<juce::FileChooser> chooser;
    MainMenuBarModel menuBarModel{*this};
    juce::MenuBarComponent menuBar;

	void codeDocumentTextInserted(const juce::String& newText, int insertIndex) override;
	void codeDocumentTextDeleted(int startIndex, int endIndex) override;
    void updateCodeDocument();
    void updateCodeEditor();

    bool keyPressed(const juce::KeyPress& key) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscirenderAudioProcessorEditor)
};
