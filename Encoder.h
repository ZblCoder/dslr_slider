/*
 Created:	5/11/2019 3:21:04 PM
 Author:	ZblCoder
*/

class Encoder : public Button
{
protected:
    int _pinA;
    int _pinB;

    int _pinALastValue;
    bool _enabledButton = true;

    void(*_onChanged)(int) = NULL;
    void(*_onDownChanged)(int) = NULL;
    
    void OnClick() override
    {
        if (_enabledButton)
            Button::OnClick();
        _enabledButton = true;
    }

    void OnLongClick() override
    {
        if (_enabledButton)
            Button::OnLongClick();
        _enabledButton = true;
    }

    void OnVeryLongClick() override
    {
        if (_enabledButton)
            Button::OnVeryLongClick();
        _enabledButton = true;
    }

    void OnChanged(int rotation)
    {
        if (_state)
        {
            _enabledButton = false;
            if (_onDownChanged != NULL)
                _onDownChanged(rotation);
        }
        else
        {
            if (_onChanged != NULL)
                _onChanged(rotation);
        }
    }
public:
    Encoder(int pinA, int pinB, int pinButton) : Button(pinButton)
    {
        _pinA = pinA;
        _pinB = pinB;

        pinMode(pinA, INPUT);
        pinMode(pinB, INPUT);

        _pinALastValue = digitalRead(_pinA);
    }
    
    void SetOnChanged(void(*onChanged)(int))
    {
        _onChanged = onChanged;
    }

    void SetOnDownChanged(void(*onDownChanged)(int))
    {
        _onDownChanged = onDownChanged;
    }

    void Update() override
    {        
        bool pinAValue = digitalRead(_pinA) == HIGH;
        if (_pinALastValue != pinAValue)
        {
            if (pinAValue)
            {
                if (digitalRead(_pinB) == HIGH)
                    OnChanged(1);
                else
                    OnChanged(-1);
            }
            _pinALastValue = pinAValue;
        }

        Button::Update();
    }
}; 
