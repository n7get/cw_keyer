#ifndef SETTINGS_H
#define SETTINGS_H

extern int wpm;
extern char ap_ssid[32];
extern char ap_password[64];
extern char sta_ssid[32];
extern char sta_password[64];
extern char radio_model[16];
extern int baud_rate;

void load_settings(void);
void register_settings_endpoints(void);

#endif // SETTINGS_H
