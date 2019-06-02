/*
 Created:	5/11/2019 3:21:04 PM
 Author:	ZblCoder
*/

class Button
{
protected:
    const int TimeLongClick = 500;
    const int TimeVeryLongClick = 3000;

    int _pin;
    bool _state;
    long _changeTime = 0;

    void(*_onClick)() = NULL;
    void(*_onLongClick)() = NULL;
    void(*_onVeryLongClick)() = NULL;
    
    virtual void OnClick()
    {
        if (_onClick != NULL)
            _onClick();
    }

    virtual void OnLongClick()
    {
        if (_onLongClick != NULL)
            _onLongClick();
    }

    virtual void OnVeryLongClick()
    {
        if (_onVeryLongClick != NULL)
            _onVeryLongClick();
    }

public:
    Button(int pin)
    {
        _pin = pin;
        pinMode(_pin, INPUT);
        
        _state = IsDown();
    }

    void SetOnClick(void(*onClick)())
    {
        _onClick = onClick;
    }

    void SetOnLongClick(void(*onLongClick)())
    {
        _onLongClick = onLongClick;
    }

    void SetOnVeryLongClick(void(*onVeryLongClick)())
    {
        _onVeryLongClick = onVeryLongClick;
    }

    bool IsDown()
    {
        return _state;
    }

    bool IsUp()
    {
        return !_state;
    }

    virtual void Update()
    {
        bool newState = digitalRead(_pin) == HIGH;
        bool isChanged = _state != newState;
        if (isChanged)
        {
            _state = newState;
            if (isChanged && !_state)
            {
                int time = millis() - _changeTime;
                if (time > TimeVeryLongClick)
                    OnVeryLongClick();
                else if (time > TimeLongClick)
                    OnLongClick();
                else
                    OnClick();
            }
            _changeTime = millis();
        }
    }
};
