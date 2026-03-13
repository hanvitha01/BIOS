#ifndef _DATE_TIME_FORM_GUID_H_
#define _DATE_TIME_FORM_GUID_H_

#define DATETIME_FORMSET_GUID \
  { 0x9a7c3211, 0x45bd, 0x4e2a, {0x88,0x12,0x66,0x44,0xaa,0xbb,0xcc,0xdd} }

extern EFI_GUID gDateTimeFormSetGuid;

#define FORM_ID_DATETIME        0x2000
#define VARSTORE_ID_DATETIME    0x3000

#define QUESTION_ID_DATE        0x4000
#define QUESTION_ID_TIME        0x4001

#endif
