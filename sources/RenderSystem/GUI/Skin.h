#pragma once
import Singleton;
import GUI;

struct Skin: public Singleton<Skin>
{
    struct Button
    {
        GUI::Texture Normal;
        GUI::Texture Hover;
        GUI::Texture Pressed;
    } DefaultButton, TabButton;

    struct Window
    {
        GUI::Texture Active;
        GUI::Texture Inactive;

    } DefaultWindow, FlowWindow, FlowComment;

    struct TabStrip
    {
        GUI::Texture Normal;

    } DefaultTabStrip;

    struct ImagePanel
    {
        GUI::Texture Normal;
		GUI::Texture Hover;

    } DefaultImagePanel;

    struct ComboBox
    {
        GUI::Texture Normal;
        GUI::Texture Hover;
        GUI::Texture Pressed;
    } DefaultComboBox;

    struct StatusBar
    {
        GUI::Texture Normal;

    } DefaultStatusBar;

    struct CheckBox
    {
        GUI::Texture Normal;
        GUI::Texture Checked;

    } DefaultCheckBox, DefaultOptionBox;

    struct EditBox
    {
        GUI::Texture Normal;
        GUI::Texture Cursor;


    } DefaultEditBox;

  struct CircleSelector
    {
        GUI::Texture Main;
        GUI::Texture Inner;
   
    } DefaultCircleSelector;

    Skin();
};

