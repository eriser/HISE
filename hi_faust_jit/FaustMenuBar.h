#ifndef __FAUST_MENU_BAR_H
#define __FAUST_MENU_BAR_H

namespace scriptnode {
namespace faust {


// Additional types for faust_jit_node
struct FaustMenuBar : public Component,
					  public ButtonListener,
					  public ComboBox::Listener

{

	FaustMenuBar(faust_jit_node *n);

	struct Factory : public PathFactory
	{
		String getId() const override { return {}; }
		juce::Path createPath(const String& url) const override;
	} factory;

	juce::ComboBox classSelector;
	HiseShapeButton addButton;
	HiseShapeButton editButton;
	HiseShapeButton reloadButton;

	WeakReference<faust_jit_node> node;
	hise::ScriptnodeComboBoxLookAndFeel claf;

	// Define menu options for addButton
	enum MenuOption {
		MENU_OPTION_FIRST = 1,
		NEW_FILE = MENU_OPTION_FIRST,
		IMPORT_FILE,
		IMPORT_LIB,
		RENAME_FILE,
		REMOVE_FILE,
		// add more options here
		MENU_OPTION_LAST,
		MENU_OPTION_INVALID,
	};

	std::map<int, String> menuOptions = {
		{NEW_FILE, "Create new file"},
		{IMPORT_FILE, "Import file into project"},
		{IMPORT_LIB, "Import library into project"},
		{RENAME_FILE, "Rename file"},
		{REMOVE_FILE, "Remove entry and file"},
			// add description for more options here
		{MENU_OPTION_INVALID, "Invalid Option (BUG)"}
	};

	String getTextForMenuOptionId(int id)
	{
		if (menuOptions.count(id) > 0) return menuOptions[id];
		return menuOptions[MENU_OPTION_INVALID];
	}

	void createNewFile();

	std::optional<File> promptForDestinationFile(String extension, File& previousDestFile);

	void importFile(String extension);

	void renameFile();

	void removeFile()
	{

	}

	void executeMenuAction(int option);


	void rebuildComboBoxItems();

	virtual void resized() override;

	virtual void buttonClicked(Button* b) override;

	virtual void comboBoxChanged (ComboBox *comboBoxThatHasChanged) override;

};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_MENU_BAR_H