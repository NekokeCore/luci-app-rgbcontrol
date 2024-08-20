module("luci.controller.rgbcontrol", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/rgbcontrol") then
		return
	end

	local e=entry({"admin", "system", "rgbcontrol"}, alias("admin", "system", "rgbcontrol", "general", "advanced"),_("RGB Control"),100)
	e.dependent = true

	entry({"admin", "system", "rgbcontrol", "general"}, cbi("rgbcontrol/general"), _("General Settings"), 20).leaf=true
	entry({"admin",  "system", "rgbcontrol", "advanced"}, cbi("rgbcontrol/advanced"), _("Advanced Settings"), 30).leaf = true
	entry({"admin", "system", "rgbcontrol", "status"}, call("rgbcontrol_status"))
end

function rgbcontrol_status()
	local sys  = require "luci.sys"
	local status = { }
	status.running = (sys.call("pidof rgbcontrol >/dev/null") == 0)
	luci.http.prepare_content("application/json")
	luci.http.write_json(status)
end