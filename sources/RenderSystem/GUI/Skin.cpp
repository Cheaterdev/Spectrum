module GUI:Skin;

import Graphics;

#define STRINGIFY(x) #x
#define PPCAT_NX(A, B) A ## B

#define GUI_TEXTURE(name) \
    EngineAsset<TextureAsset> name(PPCAT_NX(L,#name), [] { \
		return new TextureAsset(to_path(L"textures/gui/" STRINGIFY(name) ".png")); \
    });

namespace EngineAssets
{
	GUI_TEXTURE(button)
		GUI_TEXTURE(button_hover)
		GUI_TEXTURE(button_pressed)
		GUI_TEXTURE(tab_button_back)
		GUI_TEXTURE(tab_button_hover)
		GUI_TEXTURE(tab_button)
		GUI_TEXTURE(buttotab_stripn)
		GUI_TEXTURE(window_active)
		GUI_TEXTURE(combo)
		GUI_TEXTURE(combo_hover)
		GUI_TEXTURE(combo_pressed)
		GUI_TEXTURE(tab)
		GUI_TEXTURE(tab_strip)
		GUI_TEXTURE(check_normal)
		GUI_TEXTURE(check_checked)
		GUI_TEXTURE(option_normal)
		GUI_TEXTURE(option_checked)
		GUI_TEXTURE(flow_window_active)
		GUI_TEXTURE(flow_window)
		GUI_TEXTURE(flow_comment)
		GUI_TEXTURE(item_normal)
		GUI_TEXTURE(item_hover)
		GUI_TEXTURE(edit)
		GUI_TEXTURE(circle)
		GUI_TEXTURE(circle_inner)
		GUI_TEXTURE(window_close)
		GUI_TEXTURE(tree_opened)
		GUI_TEXTURE(tree_closed)
		GUI_TEXTURE(down)
		GUI_TEXTURE(background)
		GUI_TEXTURE(debug_back)
		GUI_TEXTURE(virtualtex)
		GUI_TEXTURE(shadow)
		GUI_TEXTURE(back_fill)
}

Skin::Skin()
{
	DefaultButton.Normal.texture = EngineAssets::button.get_asset()->get_texture();
	DefaultButton.Normal.padding = { 3, 3, 3, 3 };
	DefaultButton.Hover.texture = EngineAssets::button_hover.get_asset()->get_texture();
	DefaultButton.Hover.padding = { 3, 3, 3, 3 };
	DefaultButton.Pressed.texture = EngineAssets::button_pressed.get_asset()->get_texture();
	DefaultButton.Pressed.padding = { 3, 3, 3, 3 };
	TabButton.Normal.texture = EngineAssets::tab_button_back.get_asset()->get_texture();
	TabButton.Normal.padding = { 3, 3, 3, 3 };
	TabButton.Hover.texture = EngineAssets::tab_button_hover.get_asset()->get_texture();
	TabButton.Hover.padding = { 3, 3, 3, 3 };
	TabButton.Pressed.texture = EngineAssets::tab_button.get_asset()->get_texture();
	TabButton.Pressed.padding = { 3, 3, 3, 3 };
	DefaultTabStrip.Normal.texture = EngineAssets::tab_strip.get_asset()->get_texture();
	DefaultTabStrip.Normal.padding = { 0, 0, 0, 3 };
	DefaultWindow.Active.texture = EngineAssets::window_active.get_asset()->get_texture();
	DefaultWindow.Active.padding = { 11, 40, 13, 13 };
	DefaultWindow.Active.margins = { 12, 11, 11, 10 };
	DefaultComboBox.Normal.texture = EngineAssets::combo.get_asset()->get_texture();
	DefaultComboBox.Normal.padding = { 3, 3, 32, 3 };
	DefaultComboBox.Hover.texture = EngineAssets::combo_hover.get_asset()->get_texture();
	DefaultComboBox.Hover.padding = { 3, 3, 32, 3 };
	DefaultComboBox.Pressed.texture = EngineAssets::combo_pressed.get_asset()->get_texture();
	DefaultComboBox.Pressed.padding = { 3, 3, 32, 3 };
	DefaultStatusBar.Normal.texture = EngineAssets::tab.get_asset()->get_texture();
	DefaultCheckBox.Normal.texture = EngineAssets::check_normal.get_asset()->get_texture();
	DefaultCheckBox.Checked.texture = EngineAssets::check_checked.get_asset()->get_texture();
	DefaultOptionBox.Normal.texture = EngineAssets::option_normal.get_asset()->get_texture();
	DefaultOptionBox.Checked.texture = EngineAssets::option_checked.get_asset()->get_texture();
	FlowWindow.Active.texture = EngineAssets::flow_window_active.get_asset()->get_texture();
	FlowWindow.Active.padding = { 11, 40, 13, 13 };
	FlowWindow.Active.margins = { 12, 11, 11, 10 };
	FlowWindow.Inactive.texture = EngineAssets::flow_window.get_asset()->get_texture();
	FlowWindow.Inactive.padding = { 11, 40, 13, 13 };
	FlowWindow.Inactive.margins = { 12, 11, 11, 10 };
	FlowComment.Active.texture = EngineAssets::flow_comment.get_asset()->get_texture();
	FlowComment.Active.padding = { 11, 40, 13, 13 };
	FlowComment.Active.margins = { 12, 11, 11, 10 };
	DefaultImagePanel.Normal.texture = EngineAssets::item_normal.get_asset()->get_texture();
	DefaultImagePanel.Normal.padding = { 9, 9, 9, 9 };

	DefaultImagePanel.Hover.texture = EngineAssets::item_hover.get_asset()->get_texture();
	DefaultImagePanel.Hover.padding = { 9, 9, 9, 9 };



	DefaultEditBox.Normal.texture = EngineAssets::edit.get_asset()->get_texture();
	DefaultEditBox.Normal.padding = { 14, 14, 14, 14 };


	DefaultCircleSelector.Main.texture = EngineAssets::circle.get_asset()->get_texture();
	DefaultCircleSelector.Inner.texture = EngineAssets::circle_inner.get_asset()->get_texture();

	WindowCloseButton = EngineAssets::window_close.get_asset()->get_texture();
	Background = EngineAssets::background.get_asset()->get_texture();
	TreeOpened = EngineAssets::tree_opened.get_asset()->get_texture();
	TreeClosed = EngineAssets::tree_closed.get_asset()->get_texture();
	DebugBack = EngineAssets::item_hover.get_asset()->get_texture();
	DebugBack.padding = { 16,16,16,16 };
	Down = EngineAssets::down.get_asset()->get_texture();
	Virtual = EngineAssets::virtualtex.get_asset()->get_texture();
	Shadow = EngineAssets::shadow.get_asset()->get_texture();
	Fill = EngineAssets::back_fill.get_asset()->get_texture();
	Edit.padding = { 5, 5, 5, 5 };
	Virtual.padding = { 5, 5, 5, 5 };

	Shadow.padding = { 9, 9, 9, 9 };
}