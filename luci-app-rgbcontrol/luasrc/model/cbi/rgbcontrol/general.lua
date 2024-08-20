m = Map("rgbcontrol")
m.title	= translate("RGB Control")
m.description = translate("General RGB Control Settings")

s = m:section(TypedSection, "rgbcontrol", translate("Global Settings"), translate("Global settings, where the settings control the whole plugin."))
s.addremove = false
s.anonymous = true

enable=s:option(Flag, "enabled", translate("Enabled"), translate("Global setting, LEDs can only be controlled when it is enabled."))
enable.rmempty = false

s1 = m:section(TypedSection, "rgbcontrol", translate("LEDs Settings"), translate("Change the behavior of the LEDs."))
s1.addremove = false
s1.anonymous = true

leds_mode=s1:option(ListValue, "leds_mode", translate("LEDs Mode"), translate("Set leds animation."))
leds_mode:value("off", translate("OFF"))
leds_mode:value("default", translate("Default"))
leds_mode:value("breathing", translate("Breathing"))
leds_mode:value("rainbow", translate("Rainbow"))

leds_color=s1:option(Value, "leds_color", translate("Color"), translate("(Hex)Set leds color by use hex color code."))
leds_brightness=s1:option(Value, "leds_brightness", translate("Brightness"), translate("(Percentage)Set leds brightness."))
leds_brightness.datatype="range(1,100)"
return m
