#pragma once

#define INITGUID
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

typedef std::shared_ptr<IDirectInput> direct_input_ptr;
typedef std::shared_ptr<IDirectInputDevice> input_device_ptr;

class input_system final : public Singleton<input_system>
{
	input_system();
	direct_input_ptr pDirectInputSystem;
public:
	std::shared_ptr<class mouse> create_mouse(HWND hWnd);
	std::shared_ptr<class keyboard> create_keyboard(HWND hWnd);

	direct_input_ptr get_native() { return pDirectInputSystem; }

	friend Singleton<input_system>;
};

class input_device abstract
{
	input_device(const input_device&) {}
public:
	input_device(input_device_ptr pDevice);

	virtual void process_device();
protected:
	input_device_ptr pInputDevice;
};

// Mouse device
enum EMouseButtons
{
	EMB_Left,
	EMB_Right,
	EMB_Middle,

	EMB_Button4,
	EMB_Button5,
	EMB_Button6,
	EMB_Button7,
	EMB_Button8,

	EMB_Count
};

enum EButtonState
{
	EBS_Pressed,
	EBS_Moved,
	EBS_Released,
	EBS_Canceled, // when application lose focus or device, etc.

	EBS_None
};

class mouse final : public input_device
{
public:
	mouse(input_device_ptr pDevice);

	typedef std::shared_ptr<mouse> ptr;
	typedef std::function<void(EMouseButtons handle, EButtonState type, vec3 axis)> input_delegate;

	void process_device();

	/** xy - mouse location, z - wheel, if it is exist */
	vec3 get_axis() const { return vec3(mouse_state.lX, mouse_state.lY, mouse_state.lZ); }
	bool is_pressed(EMouseButtons button) const { return mouse_state.rgbButtons[static_cast<int>(button)]; }

	void register_handle(input_delegate input_handle) { input_handlers.push_back(input_handle); }
private:
	void process_inputs(EMouseButtons handle, EButtonState type, vec3 axis);
	std::vector<input_delegate> input_handlers;

	DIMOUSESTATE2 mouse_state;
};
/*
class keyboard final : public input_device
{
public:
	explicit keyboard(std::shared_ptr<IDirectInputDevice> pInput);

	bool acquire();
	void updateState();
	byte isPressed(byte button) const { return pCurState[button]&0x80; }
	bool getKeyState(byte button);
private:
	//keyboard(const keyboard&) {}

	std::shared_ptr<IDirectInputDevice> pKeyboard;
	byte* pCurState, * pLastState;
	bool* pbStates;
};*/