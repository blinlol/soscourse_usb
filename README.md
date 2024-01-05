# soscourse_usb

firstly, add info to other files

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
```

to kern/Makefrag:

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
```

to kern/init.c:

```
diff --git a/kern/init.c b/kern/init.c
index f26450b..d7cacc4 100644
--- a/kern/init.c
+++ b/kern/init.c
@@ -175,6 +175,7 @@ i386_init(void) {
 
 #if LAB >= 10
     ENV_CREATE(fs_fs, ENV_TYPE_FS);
+    ENV_CREATE(usb_usb, ENV_TYPE_FS);
 #endif
 
 #if defined(TEST)
```

to kern/pmap.c:
```
diff --git a/kern/pmap.c b/kern/pmap.c
index e586746..98d87c4 100644
--- a/kern/pmap.c
+++ b/kern/pmap.c
@@ -1192,7 +1192,8 @@ map_physical_region(struct AddressSpace *dst, uintptr_t dstart, uintptr_t pstart
             struct Page *page = page_lookup(NULL, pstart, class, PARTIAL_NODE, 1);
             if (flags & MAP_USER_MMIO) {
                 if (!page) return -E_NO_MEM;
-                if (page->refc) return -E_NO_ENT;
+                if (page->refc && !(pstart >= 0xe0000000 && pstart <= 0xe0000000 + 0x10000000)) return -E_NO_ENT;
+                if (page->refc > 1) return -E_NO_ENT;
             }
             assert(page);
             if ((res = map_page(dst, start, page, flags)) < 0) return res;
@@ -1206,7 +1207,8 @@ map_physical_region(struct AddressSpace *dst, uintptr_t dstart, uintptr_t pstart
             struct Page *page = page_lookup(NULL, pstart, class, PARTIAL_NODE, 1);
             if (flags & MAP_USER_MMIO) {
                 if (!page) return -E_NO_MEM;
-                if (page->refc) return -E_NO_ENT;
+                if (page->refc && !(pstart >= 0xe0000000 && pstart <= 0xe0000000 + 0x10000000)) return -E_NO_ENT;
+                if (page->refc > 1) return -E_NO_ENT;
             }
             assert(page);
             if ((res = map_page(dst, start, page, flags)) < 0) return res;
@@ -1218,7 +1220,6 @@ map_physical_region(struct AddressSpace *dst, uintptr_t dstart, uintptr_t pstart
 
     return 0;
}
```



Then, add a xhci-controller and a USB-device. -device nec-usb-xhci necessary for controller. You may add a virtual device (choose your one):

```
diff --git a/GNUmakefile b/GNUmakefile
index fb8c0cb..40fdf71 100644
QEMUOPTS = -hda fat:rw:$(JOS_ESP) -serial mon:stdio -gdb tcp::$(GDBPORT)
@@ -322,6 +325,12 @@ IMAGES = $(OVMF_FIRMWARE) $(JOS_LOADER) $(OBJDIR)/kern/kernel $(JOS_ESP)/EFI/BOO
 QEMUOPTS += -drive file=$(OBJDIR)/fs/fs.img,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm
 IMAGES += $(OBJDIR)/fs/fs.img
 QEMUOPTS += -bios $(OVMF_FIRMWARE)
+QEMUOPTS += -device nec-usb-xhci
+QEMUOPTS += -device usb-mouse
+QEMUOPTS += -device usb-tablet
+QEMUOPTS += -device usb-uas
+QEMUOPTS += -device usb-kbd
```




Or you can use real device.
Use lsusb to find your USB-device info. My output looks like
```
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 003: ID 13d3:5a11 IMC Networks USB2.0 VGA UVC WebCam
Bus 001 Device 004: ID 13d3:3557 IMC Networks Bluetooth Radio 
Bus 001 Device 005: ID 046d:c077 Logitech, Inc. Mouse
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```

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
```

ispras-qemu hasn't "-device usb-host" option, so you should build qemu from source code (use ./configure --enable-usb-redir --enable-libusb)
or download file "qemu-system-x86_64" from this repo and replace ispras qemu file in its directory. This file build with ubuntu 23.10

Also use sudo:

```
sudo make qemu
```

Sudo needs to provide usb access for qemu
