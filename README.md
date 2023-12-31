# soscourse_usb

write it to GNUMakefile, to compile jos with usb.

```
diff --git a/GNUmakefile b/GNUmakefile
index fb8c0cb..c7b2ae7 100644
--- a/GNUmakefile
+++ b/GNUmakefile
@@ -282,6 +282,7 @@ all: .git/hooks/post-checkout .git/hooks/pre-commit
 .PRECIOUS:  $(OBJDIR)/kern/%.o \
           $(OBJDIR)/lib/%.o $(OBJDIR)/fs/%.o $(OBJDIR)/net/%.o \
           $(OBJDIR)/user/%.o \
+          $(OBJDIR)/usb/%.o \
           $(OBJDIR)/prog/%.o
 
 KERN_CFLAGS := $(CFLAGS) -DJOS_KERNEL -DLAB=$(LAB) -mcmodel=large -m64
@@ -313,6 +314,7 @@ include prog/Makefrag
 else
 include user/Makefrag
 include fs/Makefrag
+include usb/Makefrag
 endif
'''

and to kern/Makefrag:

```
diff --git a/kern/Makefrag b/kern/Makefrag
index 0481d74..008b4c8 100644
--- a/kern/Makefrag
+++ b/kern/Makefrag
@@ -106,6 +106,7 @@ KERN_BINFILES :=    user/hello \
                        user/testfile \
                        user/icode \
                        fs/fs \
+                       usb/usb \
                        user/testfdsharing \
                        user/testpipe \
                        user/testpiperace \
'''

Use lsusb to find your USB-device info. My output looks like
```
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 003: ID 13d3:5a11 IMC Networks USB2.0 VGA UVC WebCam
Bus 001 Device 004: ID 13d3:3557 IMC Networks Bluetooth Radio 
Bus 001 Device 005: ID 046d:c077 Logitech, Inc. Mouse
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
'''

My device is mouse, I use its info 046d:c077. Add to GNUmakefile your device:

```
diff --git a/GNUmakefile b/GNUmakefile
index fb8c0cb..40fdf71 100644
QEMUOPTS = -hda fat:rw:$(JOS_ESP) -serial mon:stdio -gdb tcp::$(GDBPORT)
@@ -322,6 +325,12 @@ IMAGES = $(OVMF_FIRMWARE) $(JOS_LOADER) $(OBJDIR)/kern/kernel $(JOS_ESP)/EFI/BOO
 QEMUOPTS += -drive file=$(OBJDIR)/fs/fs.img,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm
 IMAGES += $(OBJDIR)/fs/fs.img
 QEMUOPTS += -bios $(OVMF_FIRMWARE)
+QEMUOPTS += -device nec-usb-xhci
+QEMUOPTS += -device usb-host,vendorid=0x046d,productid=0xc077
'''

then use

#sudo

```
sudo make qemu
'''

Sudo needs to provide usb access for qemu
