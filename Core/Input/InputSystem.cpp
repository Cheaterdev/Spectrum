#include "pch.h"

#pragma comment(lib, "dinput8.lib")

input_system::input_system()
{
	IDirectInput* pInput = nullptr;
	DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&pInput, nullptr);
	pDirectInputSystem = std::shared_ptr<IDirectInput>(pInput, com_deleter);
}

std::shared_ptr<mouse> input_system::create_mouse(HWND hWnd)
{
	IDirectInputDevice* pMouse = nullptr;
	
	pDirectInputSystem->CreateDevice(GUID_SysMouse, &pMouse, nullptr);
	pMouse->SetDataFormat(&c_dfDIMouse2);
	pMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	return std::make_shared<mouse>(input_device_ptr(pMouse, com_deleter));
}
/*
std::shared_ptr<keyboard> input_system::create_keyboard(HWND hWnd)
{
	IDirectInputDevice* pKeyboard = nullptr;

	pDirectInputSystem->CreateDevice(GUID_SysKeyboard, &pKeyboard, nullptr);
	pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	return std::make_shared<keyboard>(pKeyboard);
}*/

input_device::input_device(input_device_ptr pDevice) : pInputDevice(pDevice)
{
	pInputDevice->Acquire();
}

void input_device::process_device()
{}

mouse::mouse(input_device_ptr pDevice) : input_device(pDevice)
{
	ZeroMemory(&mouse_state, sizeof(DIMOUSESTATE2));
}

void mouse::process_device()
{
	input_device::process_device();

	static bool g_bInputCanceled = false;
	HRESULT hr;

	DIMOUSESTATE2 new_mouse_state;
	ZeroMemory(&new_mouse_state, sizeof(DIMOUSESTATE2));
	hr = pInputDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &new_mouse_state);
	if (FAILED(hr))
	{
		hr = pInputDevice->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = pInputDevice->Acquire();
		
		if (!g_bInputCanceled)
		{
			// Notify about input lose
			for (int iButton = 0; iButton < EMB_Count; ++iButton)
			{
				if (is_pressed(EMouseButtons(iButton)))
					process_inputs(EMouseButtons(iButton), EBS_Canceled, { 0, 0, 0 });
			}
			g_bInputCanceled = true;
		}
		return;
	}

	// reset cancel flag
	g_bInputCanceled = false;

	// check state for every button
	vec3 axis = { new_mouse_state.lX, new_mouse_state.lY, new_mouse_state.lZ };
	for (int iButton = 0; iButton < EMB_Count; ++iButton)
	{
		bool bEqual = mouse_state.rgbButtons[iButton] == new_mouse_state.rgbButtons[iButton];
		bool bGreater = mouse_state.rgbButtons[iButton] < new_mouse_state.rgbButtons[iButton];

		EButtonState type = EBS_None;
		if (!bEqual)
		{
			if (bGreater)
				type = EBS_Pressed;
			else
				type = EBS_Released;
		}
		else
		{
			if (is_pressed(EMouseButtons(iButton)))
				type = EBS_Moved;
		}

		if (type != EBS_None && // if pressed
			(type != EBS_Moved || (get_axis() - axis).length_squared()>0)) // if moved
			process_inputs(EMouseButtons(iButton), type, axis);
	}

	mouse_state = new_mouse_state;
}

void mouse::process_inputs(EMouseButtons handle, EButtonState type, vec3 axis)
{
	for (auto& input_handle : input_handlers)
		input_handle(handle, type, axis);
}

/*
keyboard::keyboard(IDirectInputDevice* pInput)
{
	pKeyboard = std::shared_ptr<IDirectInputDevice>(pInput, com_deleter);;

	pbStates = new bool[3*256];
	pCurState = (byte*)(pbStates + 256);
	pLastState = (byte*)(pbStates + 2*256);
	ZeroMemory(pbStates, 3*256*sizeof(bool));
}

bool keyboard::acquire()
{
	if (FAILED(pKeyboard->Acquire()))
	{
		ZeroMemory(pLastState, 256*sizeof(byte));
		return false;
	}

	return true;
}

void keyboard::updateState()
{
	swap(pCurState, pLastState);
	pKeyboard->GetDeviceState(256*sizeof(byte), pCurState);
}

bool keyboard::getKeyState(byte button)
{
	if ((pCurState[button]&0x80) && ((pLastState[button]&0x80) == 0))
	{
		pLastState[button] = 0x80;
		pbStates[button] = !pbStates[button];
	}
	
	return pbStates[button];
}*/