digi_pins*	digi_serial::_pins;
short		digi_serial::_tx_pin;
short		digi_serial::_rx_pin;
short		digi_serial::_tx_inter;
short		digi_serial::_rx_inter;
bool		digi_serial::_write_busy;
QueueList<bool> digi_serial::_in_queue;

#if (DRONE == 0)
#else
char		digi_batt::_current_contr;
float		digi_batt::_percentage_contr;
float		digi_batt::_voltage_contr;
short		digi_batt::_bound_ch0;
short		digi_batt::_bound_ch1;
short		digi_batt::_bound_ch2;
short		digi_batt::_bound_ch3;
short		digi_batt::_bound_ch4;
void		(*digi_batt::_onalert)();
#endif

// My special chars locations
#define SP_CHR_0 (char)8
#define SP_CHR_1 (char)9
#define SP_CHR_2 (char)10
#define SP_CHR_3 (char)11
#define SP_CHR_4 (char)12
#define SP_CHR_5 (char)13
#define SP_CHR_6 (char)14
#define SP_CHR_7 (char)15
digi_pins	*digi_rf::_pins;
digi_lcd	*digi_rf::_screen;
void		(*digi_rf::_onload)();
int			digi_rf::_inter_in;
int			digi_rf::_neg_status=1;
int			digi_rf::_neg_status_2=0;
bool		digi_rf::_neg_tmp_data[20];
int			digi_rf::_in_pins[4];
int			digi_rf::_out_pins[4];