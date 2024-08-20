include $(TOPDIR)/rules.mk

PKG_NAME:=rgbcontrol
PKG_VERSION:=1
PKG_RELEASE:=5


PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/rgbcontrol
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=RGBControl for QWRT
endef

define Package/rgbcontrol/description
  RGBControl for QWRT.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

MAKE_FLAGS += \
	-C $(PKG_BUILD_DIR)

define Package/rgbcontrol/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rgbcontrol $(1)/usr/bin/rgbcontrol
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/rgbcontrol.config $(1)/etc/config/rgbcontrol
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rgbcontrol.init $(1)/etc/init.d/rgbcontrol
endef

$(eval $(call BuildPackage,rgbcontrol))
