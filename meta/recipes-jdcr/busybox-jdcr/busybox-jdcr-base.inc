inherit cml1 systemd

# kern-tools-native provides merge_config.sh used in do_configure
# virtual/crypt provides libcrypt for login/passwd applets
DEPENDS += "kern-tools-native virtual/crypt"

####################################################################################################
# #########                       do_configure part 
####################################################################################################

# internal helper
def busybox_cfg(feature, tokens, cnf, rem):
    if type(tokens) == type(""):
        tokens = [tokens]
    rem.extend(['/^[# ]*' + token + '[ =]/d' for token in tokens])
    if feature:
        cnf.extend([token + '=y' for token in tokens])
    else:
        cnf.extend(['# ' + token + ' is not set' for token in tokens])

# Map distro features to config settings
def features_to_busybox_settings(d):
    cnf, rem = ([], [])
    busybox_cfg(bb.utils.contains('DISTRO_FEATURES', 'ipv6', True, False, d), 'CONFIG_FEATURE_IPV6', cnf, rem)
    busybox_cfg(True, 'CONFIG_LFS', cnf, rem)
    busybox_cfg(True, 'CONFIG_FDISK_SUPPORT_LARGE_DISKS', cnf, rem)
    busybox_cfg(bb.utils.contains('DISTRO_FEATURES', 'nls', True, False, d), 'CONFIG_LOCALE_SUPPORT', cnf, rem)
    busybox_cfg(bb.utils.contains('DISTRO_FEATURES', 'ipv4', True, False, d), 'CONFIG_FEATURE_IFUPDOWN_IPV4', cnf, rem)
    busybox_cfg(bb.utils.contains('DISTRO_FEATURES', 'ipv6', True, False, d), 'CONFIG_FEATURE_IFUPDOWN_IPV6', cnf, rem)
    busybox_cfg(bb.utils.contains_any('DISTRO_FEATURES', 'bluetooth wifi', True, False, d), 'CONFIG_RFKILL', cnf, rem)
    busybox_cfg(bb.utils.contains_any('DISTRO_FEATURES', 'HELLOJDCR_FEATURE', True, False, d), 'CONFIG_HELLOJDCR', cnf, rem)
    return "\n".join(cnf), "\n".join(rem)

def features_to_busybox_conf(d):
    cnf, rem = features_to_busybox_settings(d)
    return cnf
def features_to_busybox_del(d):
    cnf, rem = features_to_busybox_settings(d)
    return rem

configmangle = '/CONFIG_EXTRA_CFLAGS/d; \
		'

# List of features, with their string as "feature=y"
OE_FEATURES := "${@features_to_busybox_conf(d)}"

# list of features, with string "feature="
OE_DEL      := "${@features_to_busybox_del(d)}"
DO_IPv4 := "${@bb.utils.contains('DISTRO_FEATURES', 'ipv4', 1, 0, d)}"
DO_IPv6 := "${@bb.utils.contains('DISTRO_FEATURES', 'ipv6', 1, 0, d)}"

python () {
  if "${OE_DEL}":
    d.setVar('configmangle:append', "${OE_DEL}" + "\n")
  if "${OE_FEATURES}":
    d.setVar('configmangle:append',
                   "/^### DISTRO FEATURES$/a\\\n%s\n\n" %
                   ("\\n".join((d.expand("${OE_FEATURES}").split("\n")))))
  d.setVar('configmangle:append',
                 "/^### CROSS$/a\\\n%s\n" %
                  ("\\n".join(["CONFIG_EXTRA_CFLAGS=\"${CFLAGS} ${HOST_CC_ARCH}\""
                        ])
                  ))
}


do_prepare_config () {

  # dont write timestamp 
	export KCONFIG_NOTIMESTAMP=1

  # remove the lines from defconfig, containing "CONFIG_STATIC" string
  # and save file as .config. 
	sed -e '/CONFIG_STATIC/d' \
		< ${WORKDIR}/defconfig > ${S}/.config

  # in .config file, Add line "# CONFIG_STATIC is not set"
  # static compilation is turned off for busybox. 
	echo "# CONFIG_STATIC is not set" >> .config

  # append 2 lines at end of .config file. 
  # "### CROSS"
  # "### DISTRO FEATURES"
	for i in 'CROSS' 'DISTRO FEATURES'; do echo "### $i"; done >> \
		${S}/.config
  
  # run the configmangle sed script, which deleted the strong present in OE_DEL
  # adds it back again using OE_DISTRO_FEATURES
	sed -i -e '${configmangle}' ${S}/.config

  # if DO_IPVv4 and DO_IPv6 disabled, then 
  # disabled the network utilities. 
  # making tmp file, as awk writes in that,
  # then we copy tmp file to main .config file. 
	if test ${DO_IPv4} -eq 0 && test ${DO_IPv6} -eq 0; then
		# disable networking applets
		mv ${S}/.config ${S}/.config.oe-tmp
		awk 'BEGIN{net=0}
		/^# Networking Utilities/{net=1}
		/^#$/{if(net){net=net+1}}
		{if(net==2&&$0 !~ /^#/&&$1){print("# "$1" is not set")}else{print}}' \
		${S}/.config.oe-tmp > ${S}/.config
	fi

	sed -i 's/CONFIG_IFUPDOWN_UDHCPC_CMD_OPTIONS="-R -n"/CONFIG_IFUPDOWN_UDHCPC_CMD_OPTIONS="-R -b"/' ${S}/.config
	if [ -n "${DEBUG_PREFIX_MAP}" ]; then
		sed -i 's|${DEBUG_PREFIX_MAP}||g' ${S}/.config
	fi
}

# function which will copy original files,
# so we can diff it later
copy_orginal_files() {
  tmp_file="${WORKDIR}/jdcr-configure_files"
  mkdir -p ${tmp_file}
  cp ${WORKDIR}/defconfig ${tmp_file}/
  cp ${S}/.config ${tmp_file}/
}

do_configure () {
	set -x
	do_prepare_config
  copy_orginal_files
  
  # run merge_config, merging all configs with other configs with d. 
	merge_config.sh -m .config ${@" ".join(find_cfgs(d))}
  cp .config ${WORKDIR}/jdcr-configure_files/after_merge_config.config
  # running cm1_do_configure
  # filling in defaults for anything which is missing. 
	cml1_do_configure

	# Save a copy of .config and autoconf.h.
	cp .config .config.orig
	cp include/autoconf.h include/autoconf.h.orig
}

####################################################################################################


####################################################################################################
#                                 do_compile 
####################################################################################################

BUSYBOX_SPLIT_SUID ?= "1"

export EXTRA_CFLAGS = "${CFLAGS}"
export EXTRA_LDFLAGS = "${LDFLAGS}"

EXTRA_OEMAKE = "CC='${CC}' LD='${CCLD}' V=1 ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_PREFIX} SKIP_STRIP=y HOSTCC='${BUILD_CC}' HOSTCPP='${BUILD_CPP}'"


# we need to define do_compile from scratch, as 
# we need to do suid and all stuff not just compilation. 
do_compile(){

  bbnote "HERE in do_compile"
  unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS
	export KCONFIG_NOTIMESTAMP=1

  # not in normal flow 
  cp .config.orig .config
	cp include/autoconf.h.orig include/autoconf.h

  # BUSYBOX_SPLIT_SUID is bitbake variable. 
  # CONFIG_FEATURE_INDIVIDUAL is defined in .config file and we can set it. 
  # condition will be true, when we want to split & CONFIG_FEATURE_INDIVIDUAL is not enabled.
  if [ "${BUSYBOX_SPLIT_SUID}" = "1" -a x`grep "CONFIG_FEATURE_INDIVIDUAL=y" .config` = x ]; then
    
    # clean temporary files.
		rm -f .config.app.suid .config.app.nosuid .config.disable.apps .config.nonapps

    # split the .config into two parts, and make two busybox binaries
    # will make 2 files, suid : command requiring super user 
    # nosuid : command not needing super user access. 
		oe_runmake busybox.cfg.suid
		oe_runmake busybox.cfg.nosuid

    # NOT in NORMAL FLOW 
    # workaround for suid bug 10346
		if ! grep -q "CONFIG_SH_IS_NONE" busybox.cfg.nosuid; then
			echo "CONFIG_SH_IS_NONE" >> busybox.cfg.suid
		fi

    # paste the contents in given files, 
    # with comments. 
		for i in `cat busybox.cfg.suid busybox.cfg.nosuid`; do
			echo "# $i is not set" >> .config.disable.apps
		done

    # merge config.disable.apps to config.orig
    merge_config.sh -m .config.orig .config.disable.apps
    cp .config .config.nonapps
    for s in suid nosuid; do
			cat busybox.cfg.$s | while read item; do
				grep -w "$item" .config.orig
			done > .config.app.$s

			# workaround for suid bug 10346
			if [ "$s" = "suid" ] ; then
				sed "s/.*CONFIG_SH_IS_NONE.*$/CONFIG_SH_IS_NONE=y/" -i .config.app.suid
			fi

			merge_config.sh -m .config.nonapps .config.app.$s
			oe_runmake busybox_unstripped
			mv busybox_unstripped busybox.$s
			oe_runmake busybox.links
			sort busybox.links > busybox.links.$s
			rm busybox.links
		done

		# hard fail if sh is being linked to the suid busybox (detects bug 10346)
		if grep -q -x "/bin/sh" busybox.links.suid; then
			bbfatal "busybox suid binary incorrectly provides /bin/sh"
		fi

		# cleanup
		rm .config.app.suid .config.app.nosuid .config.disable.apps .config.nonapps


  else
    bbnote "building busybox unstripped"
    oe_runmake busybox_unstripped
    oe_runmake busybox.links
  fi

  # not in normal flow 
  cp .config.orig .config
	cp include/autoconf.h.orig include/autoconf.h

}

####################################################################################################

do_install() {

  # in .link files, replace ^/bin/ with BASE_BINDIR
  sed -i "s:^/bin/:BASE_BINDIR/:" busybox.links*
	sed -i "s:^/sbin/:BASE_SBINDIR/:" busybox.links*
	sed -i "s:^/usr/bin/:BINDIR/:" busybox.links*
	sed -i "s:^/usr/sbin/:SBINDIR/:" busybox.links*

	# Move arch/link to BINDIR to match coreutils
	sed -i "s:^BASE_BINDIR/arch:BINDIR/arch:" busybox.links*
	sed -i "s:^BASE_BINDIR/link:BINDIR/link:" busybox.links*
  
	sed -i "s:^BASE_BINDIR/:${base_bindir}/:" busybox.links*
	sed -i "s:^BASE_SBINDIR/:${base_sbindir}/:" busybox.links*
	sed -i "s:^BINDIR/:${bindir}/:" busybox.links*
	sed -i "s:^SBINDIR/:${sbindir}/:" busybox.links*

  # creates the directory /etc/init.d under ${D}
  install -d ${D}${sysconfdir}/init.d

  if ! grep -q "CONFIG_FEATURE_INDIVIDUAL=y" ${B}/.config; then
    
    # create directory base_bindir under ${D}
    install -d ${D}${base_bindir}
    if [ "${BUSYBOX_SPLIT_SUID}" = "1" ]; then

      bbnote "\"BUSYBOX_SPLIT_SUIT\" == 1"

      # install the binaries. and also links in /etc folder. 
			install -m 4755 ${B}/busybox.suid ${D}${base_bindir}
			install -m 0755 ${B}/busybox.nosuid ${D}${base_bindir}
			install -m 0644 ${S}/busybox.links.suid ${D}${sysconfdir}
			install -m 0644 ${S}/busybox.links.nosuid ${D}${sysconfdir}
			
      if grep -q "CONFIG_SH_IS_ASH=y" ${B}/.config; then
				ln -sf busybox.nosuid ${D}${base_bindir}/sh
			fi
			
      # Keep a default busybox for people who want to invoke busybox directly.
			# This is also useful for the on device upgrade. Because we want
			# to use the busybox command in postinst.
			ln -sf busybox.nosuid ${D}${base_bindir}/busybox

    else
      bbnote "\"BUSYBOX_SPLIT_SUIT\" != 1"

      # as splitting is disabled, we look, which variant we want to load. 
      # suid or nosuid. and according to it, we install in bindir. 
      if grep -q "CONFIG_FEATURE_SUID=y" ${B}/.config; then
				install -m 4755 ${B}/busybox ${D}${base_bindir}
			else
				install -m 0755 ${B}/busybox ${D}${base_bindir}
			fi

			install -m 0644 ${S}/busybox.links ${D}${sysconfdir}
			if grep -q "CONFIG_SH_IS_ASH=y" ${B}/.config; then
				ln -sf busybox ${D}${base_bindir}/sh
			fi
			ln -sf busybox ${D}${base_bindir}/busybox.nosuid
    fi
  else
    bbnote "CONFIG_FEATURE_INDIVIDUAL != y"
    # Not in normal flow. still needs to be come back to. 
    # TODO : come back to this. flow later. 
  fi
}

####################################################################################################

inherit update-alternatives
ALTERNATIVE_PRIORITY = "50"
do_package:prepend(){
  
    dvar = d.getVar('D')
    pn = d.getVar('PN')
    def set_alternative_variables (link, target): 
      links = d.expand (link)
      target = d.expand (target)
      f = open('%s%s' % (dvar, links), 'r')
      
      for alt_link_name in f:
        alt_link_name = alt_link_name.strip()
        alt_name = os.path.basename(alt_link_name)

        # below part is there to match coreutils. 
        if alt_name == '[':
                alt_name = 'lbracket'
        if alt_name == 'klogd' or alt_name == 'syslogd':
            d.appendVar('ALTERNATIVE:%s-syslog' % (pn), ' ' + alt_name)
        else:
            # normal flow for most of stuff.
            # will append the command name like ls, cat in ALTERNATIVE:busybox
            d.appendVar('ALTERNATIVE:%s' % (pn), ' ' + alt_name)
        
        # add new variable in yocto datastore, ALTERNATIVE_LINK_NAME[ls] = {path to ls}
        d.setVarFlag('ALTERNATIVE_LINK_NAME', alt_name, alt_link_name)
        bb.note(f"ALTERNATIVE_LINK_NAME[{alt_name}] : {alt_link_name}")
        if os.path.exists('%s%s' % (dvar, target)):
            d.setVarFlag('ALTERNATIVE_TARGET', alt_name, target)
            bb.note(f"ALTERNATIVE_TARGET[{alt_name}] : {d.getVarFlag('ALTERNATIVE_TARGET', alt_name)}")
       
      f.close()
      bb.note(f"ALTERNATIVE:{pn} : {d.getVar('ALTERNATIVE:' + pn)}")
      return
    if os.path.exists('%s/etc/busybox.links' % (dvar)):
        set_alternative_variables("${sysconfdir}/busybox.links", "${base_bindir}/busybox")
    else:
        set_alternative_variables("${sysconfdir}/busybox.links.nosuid", "${base_bindir}/busybox.nosuid")
        set_alternative_variables("${sysconfdir}/busybox.links.suid", "${base_bindir}/busybox.suid")
      
}

# find the backup folder and fillup global variable with the folder location
pkg_postinst:${PN}:prepend () {
        # Need path to saved utils, but they may have be removed on upgrade of busybox
        # Only use shell to get paths. Also capture if busybox was saved.
        BUSYBOX=""
        if [ "x$D" = "x" ] ; then 
           for busybox_rmdir in /tmp/busyboxrm-*; do
               if [ "$busybox_rmdir" != '/tmp/busyboxrm-*' ] ; then
                  export PATH=$busybox_rmdir:$PATH
                  if [ -e $busybox_rmdir/busybox* ] ; then
                    BUSYBOX="$busybox_rmdir/busybox*"
                  fi
               fi
           done
        fi
}


# backup folder and delete it. 
# first do update-alternative, to remove files
# then remove backup folder. 
pkg_postinst:${PN}:append () {
        # If busybox exists in the remove directory it is because it was the only shell left.
        if [ "x$D" = "x" ] ; then
           if [ "x$BUSYBOX" != "x" ] ; then
              update-alternatives --remove sh $BUSYBOX
              rm -f $BUSYBOX
           fi
        fi
} 