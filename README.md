# FADCDAQ
Codes for NOTICE FADC DAQ operation

//-----------------------------------------------

Apr. 2, 2025:
- Items in the inventory\
	a. DAQ:\
		a-1. NKFADC500 (4 ch/module) x2 (2 modules in a form factor)\
		a-2. JBNU_DAQ (32 ch/module) x2\
		a-3. BIC_DAQ (32 ch/module) x2
	b. TCB:\
		b-1. minitcb_v2 (4 slots)\
		b-2. BIC_TCB (40 slots)
- Existing packages\
	c. minitcb_fadc_jbnu_sa: minitcb + NKFADC or JBNU_DAQ, in stand-alone mode\
	d. minitcb_fadc_jbnu_ext: minitcb + NKFADC + JBNU_DAQ, using external trigger\
	e. tcb_fadc_jbnu_bic_ext: BIC_TCB + NKFADC + JBNU_DAQ +BIC_DAQ (APix modified), using ext. trigger
- Common requirements before installation\
	f. ROOT (> v6.x)\
	g. libusb (in ubuntu prompt, try "apt-get install libusb-1.0.0.dev")
- Installation\
	h. Make sure ROOT and libusb are installed\
		h-1. ROOTHOME: try command "echo $ROOTSYS" to check installed ROOT location\
		h-2. LIBUSB_INC: in general, "libusb.h" can be found under "/usr/include/libusb-1.0/"\
		h-3. LIBUSB_LIB: in general, "libusb-1.0.*" can be found under "/usr/lib/x86_64-linux-gnu/"
	i. In a package, specify previous 3 items' (ROOTHOME, LIBUSB_INC, LIBUSB_LIB) path in the "notice_env.sh"\
		* remember this "notice_env.sh" must be executed once before using the package
	j. If above h. and i. are properly set, then execute "notice_install.sh"\
		* check "include" and "lib" have newly compiled libraries
	k. Put file "71-notice-permissions.rules" under "/etc/udev/rules.d/" (need super user permission)
- Operation\
	l. Enter the package you're going to use\
	m. Compile operation codes: use either command "make" or script "./Make.sh"\
	n. 3 scripts control the operation: INIT.sh (initialization), RUN.sh (running), and STOP.sh
	
