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
