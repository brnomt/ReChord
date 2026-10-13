/* usb_cdc.h — USB CDC-ACM (virtual serial) debug console (Echo Mini). */
#ifndef RECHORD_USB_CDC_H
#define RECHORD_USB_CDC_H

/* Init the CDC-ACM device (DWC2 + enumeration). */
void usb_cdc_init(void);

/* Poll one iteration (EP0 setup + bulk OUT). Call in a loop. */
void usb_cdc_poll(void);

/* Run the CDC-ACM device + poll loop until usb_cdc_request_exit(). */
int  usb_cdc_run(void);

/* Send text to the host terminal (bulk IN). */
int  usb_cdc_puts(const char *str);

/* Receive: next byte from the host, or -1 if none pending. */
int  usb_cdc_getc(void);
int  usb_cdc_available(void);

/* Ask usb_cdc_run() to return. */
void usb_cdc_request_exit(void);

#endif /* RECHORD_USB_CDC_H */
