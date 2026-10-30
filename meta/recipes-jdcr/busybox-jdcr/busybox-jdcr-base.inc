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
    return "\n".join(cnf), "\n".join(rem)

def features_to_busybox_conf(d):
    cnf, rem = features_to_busybox_settings(d)
    bb.note("JDCR :: cnf list : {}", cnf);
    return cnf
def features_to_busybox_del(d):
    cnf, rem = features_to_busybox_settings(d)
    bb.note("JDCR :: rem list : {}", rem);
    return rem

configmangle = '/CONFIG_EXTRA_CFLAGS/d; \
		'
OE_FEATURES := "${@features_to_busybox_conf(d)}"
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