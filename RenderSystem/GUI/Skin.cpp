#include "pch.h"


Skin::Skin()
{
    DefaultButton.Normal.texture = Render::Texture::get_resource({ "textures/gui/button.png", false, false });
    DefaultButton.Normal.padding = { 3, 3, 3, 3 };
    DefaultButton.Hover.texture = Render::Texture::get_resource({ "textures/gui/button_hover.png", false, false });
    DefaultButton.Hover.padding = { 3, 3, 3, 3 };
    DefaultButton.Pressed.texture = Render::Texture::get_resource({ "textures/gui/button_pressed.png", false, false });
    DefaultButton.Pressed.padding = { 3, 3, 3, 3 };
    TabButton.Normal.texture = Render::Texture::get_resource({ "textures/gui/tab_button_back.png", false, false });
    TabButton.Normal.padding = { 3, 3, 3, 3 };
    TabButton.Hover.texture = Render::Texture::get_resource({ "textures/gui/tab_button_hover.png", false, false });
    TabButton.Hover.padding = { 3, 3, 3, 3 };
    TabButton.Pressed.texture = Render::Texture::get_resource({ "textures/gui/tab_button.png", false, false });
    TabButton.Pressed.padding = { 3, 3, 3, 3 };
    DefaultTabStrip.Normal.texture = Render::Texture::get_resource({ "textures/gui/tab_strip.png", false, false });
    DefaultTabStrip.Normal.padding = { 0, 0, 0, 3 };
    DefaultWindow.Active.texture = Render::Texture::get_resource({ "textures/gui/window_active.png", false, false });
    DefaultWindow.Active.padding = { 11, 40, 13, 13 };
    DefaultWindow.Active.margins = { 12, 11, 11, 10 };
    DefaultComboBox.Normal.texture = Render::Texture::get_resource({ "textures/gui/combo.png", false, false });
    DefaultComboBox.Normal.padding = { 3, 3, 32, 3 };
    DefaultComboBox.Hover.texture = Render::Texture::get_resource({ "textures/gui/combo_hover.png", false, false });
    DefaultComboBox.Hover.padding = { 3, 3, 32, 3 };
    DefaultComboBox.Pressed.texture = Render::Texture::get_resource({ "textures/gui/combo_pressed.png", false , false });
    DefaultComboBox.Pressed.padding = { 3, 3, 32, 3 };
    DefaultStatusBar.Normal.texture = Render::Texture::get_resource({ "textures/gui/tab.png", false, false });
    DefaultCheckBox.Normal.texture = Render::Texture::get_resource({ "textures/gui/check_normal.png", false , false });
    DefaultCheckBox.Checked.texture = Render::Texture::get_resource({ "textures/gui/check_checked.png", false , false });
    DefaultOptionBox.Normal.texture = Render::Texture::get_resource({ "textures/gui/option_normal.png", false , false });
    DefaultOptionBox.Checked.texture = Render::Texture::get_resource({ "textures/gui/option_checked.png", false , false });
    FlowWindow.Active.texture = Render::Texture::get_resource({ "textures/gui/flow_window_active.png", false, true });
    FlowWindow.Active.padding = { 11, 40, 13, 13 };
    FlowWindow.Active.margins = { 12, 11, 11, 10 };
    FlowWindow.Inactive.texture = Render::Texture::get_resource({ "textures/gui/flow_window.png", false, true });
    FlowWindow.Inactive.padding = { 11, 40, 13, 13 };
    FlowWindow.Inactive.margins = { 12, 11, 11, 10 };
    FlowComment.Active.texture = Render::Texture::get_resource({ "textures/gui/flow_comment.png", false, true });
    FlowComment.Active.padding = { 11, 40, 13, 13 };
    FlowComment.Active.margins = { 12, 11, 11, 10 };
    DefaultImagePanel.Normal.texture = Render::Texture::get_resource({ "textures/gui/item_normal.png", false, true });
    DefaultImagePanel.Normal.padding = { 9, 9, 9, 9 };

	DefaultImagePanel.Hover.texture = Render::Texture::get_resource({ "textures/gui/item_hover.png", false, true });
	DefaultImagePanel.Hover.padding = { 9, 9, 9, 9 };



    DefaultEditBox.Normal.texture = Render::Texture::get_resource({ "textures/gui/edit.png", false, true });
    DefaultEditBox.Normal.padding = {14, 14, 14, 14};


	DefaultCircleSelector.Main.texture = Render::Texture::get_resource({ "textures/gui/circle.png", false, true });
	DefaultCircleSelector.Inner.texture = Render::Texture::get_resource({ "textures/gui/circle_inner.png", false, true });


}