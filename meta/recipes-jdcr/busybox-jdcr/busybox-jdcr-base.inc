inherit cml1 systemd

## configuration part 

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