
#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gl-rs485
PKG_VERSION:=1.1.0
PKG_RELEASE:=1
include $(INCLUDE_DIR)/package.mk


define Package/gl-rs485/Default
	SECTION:=base
	CATEGORY:=gl-inet
	TITLE:=GL iNet rs485 1.0
	DEPENDS:=+libblobmsg-json  +gl-util +libuci +libjson-c  +mqtt
endef

Package/gl-rs485 = $(Package/gl-rs485/Default)

define Package/gl-rs485/description
gl rs485 for gl-inet.
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/gl-rs485/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gl-rs485 $(1)/usr/bin
	
	$(INSTALL_DIR) $(1)/usr/lib/gl
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/librs485api.so $(1)/usr/lib/gl
	$(LN) /usr/lib/gl/librs485api.so $(1)/usr/lib/		
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./file/rs485 $(1)/etc/config/rs485
endef

$(eval $(call BuildPackage,gl-rs485))
