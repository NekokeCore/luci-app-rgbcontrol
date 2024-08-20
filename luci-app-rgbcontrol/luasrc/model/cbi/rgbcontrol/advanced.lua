m = Map("rgbcontrol")
m.title	= translate("RGB Control")
m.description = translate("Advanced RGB Control Settings")

s = m:section(TypedSection, "rgbcontrol" ,translate("PWM Chip Settings"), translate("Global PWM Chip Settings"))
s.addremove = false
s.anonymous = true

pwm_path = s:option(Value, "pwm_path", translate("PWM Chip Path"), translate("(Path)Set the path of PWM."))
pwm_name = s:option(Value, "pwm_name", translate("RGB PWM Chip Name"), translate("Set the name of the chip where the RGB is located."))

s1 = m:section(TypedSection, "rgbcontrol" ,translate("LEDs PWM Settings"), translate("Global LEDs PWM Settings"))
s1.addremove = false
s1.anonymous = true

period = s1:option(Value, "period", translate("LED PWM Period"), translate("Set the period for LEDs"))
red_pwm_pin = s1:option(Value, "red_pwm_name", translate("Red LED PWM Pin"), translate("Set the pin name of the Red LED on the PWM chip."))
green_pwm_pin = s1:option(Value, "green_pwm_name", translate("Green LED PWM Pin"), translate("Set the pin name of the Green LED on the PWM chip."))
blue_pwm_pin = s1:option(Value, "blue_pwm_name", translate("Blue LED PWM Pin"), translate("Set the pin name of the Blue LED on the PWM chip."))

s2 = m:section(TypedSection, "rgbcontrol" ,translate("LEDs Tweak Settings"), translate("Global LEDs Tweak Settings"))
s2.addremove = false
s2.anonymous = true

red_led_tweak = s2:option(Value, "red_led_tweak", translate("Red LED Tweak"), translate("(Number)Caution! Set the color calibration value for the Red LED."))
red_led_tweak.datatype="range(1,255)"
green_led_tweaks = s2:option(Value, "green_led_tweak", translate("Green LED Tweak"), translate("(Number)Caution! Set the color calibration value for the Green LED."))
green_led_tweaks.datatype="range(1,255)"
blue_led_tweaks = s2:option(Value, "blue_led_tweak", translate("Blue LED Tweak"), translate("(Number)Caution! Set the color calibration value for the Blue LED."))
blue_led_tweaks.datatype="range(1,255)"

return m
