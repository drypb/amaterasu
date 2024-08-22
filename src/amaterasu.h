#ifndef AMATERASU_H
#define AMATERASU_H

struct _DRIVER_SETTINGS {
    
    PDRIVER_OBJECT  DriverObj;
    PUNICODE_STRING RegistryPath;

    union {
        BOOLEAN FltArr[];
        struct {
            BOOLEAN FSFltr;
            BOOLEAN ProcFltr;
        } Filter;
    } Loaded;
};

typedef struct _DRIVER_SETTINGS DRIVER_SETTINGS, *PDRIVER_SETTINGS;

struct _AMATERASU {

    DRIVER_SETTINGS Settings;

    union {
        PVOID FltArr[];
        struct {
            PFSFLTR   FSFltr;
            PPROCFLTR ProcFltr;
        } Filter;
    } Filters;
};

typedef struct _AMATERASU AMATERASU, *PAMATERASU;

#endif  /* AMATERASU_H */
