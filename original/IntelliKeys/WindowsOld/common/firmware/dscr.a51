;; -------------------------------------------------------------- FILE: Dscr.a51
;;     FILE:	Dscr.a51
;;   AUTHOR:	D. Cooper
;; REVISION:	1.0		 8-01-00		Created.
;; COMMENTS:	Note that multi-byte values are expressed in little endian.
;;
;; fwr	11/22/00	10-msec polling
;; fwr  12/1/00         100-mAmp power
;; JWH  11/2/02		Added two additional HID interfaces
;; 			Disable by setting JWH to 0
$set (JWH)
$set (FRED_MOUSE_REPORT)
;;
;; -----------------------------------------------------------------------------
;;

DSCR_DEVICE	equ	1	;; Descriptor type: Device
DSCR_CONFIG	equ	2	;; Descriptor type: Configuration
DSCR_STRING	equ	3	;; Descriptor type: String
DSCR_INTRFC	equ	4	;; Descriptor type: Interface
DSCR_ENDPNT	equ	5	;; Descriptor type: Endpoint

ET_CONTROL	equ	0	;; Endpoint type: Control
ET_ISO		equ	1	;; Endpoint type: Isochronous
ET_BULK		equ	2	;; Endpoint type: Bulk
ET_INT		equ	3	;; Endpoint type: Interrupt

public		DeviceDscr, ConfigDscr, StringDscr, UserDscr, ReportDscr, ReportDscrLen
public		HIDDscr

DSCR	SEGMENT	CODE

;;-----------------------------------------------------------------------------
;; Global Variables
;;-----------------------------------------------------------------------------
		rseg DSCR		;; locate the descriptor table in on-part memory.

DeviceDscr:	
		db	18		;; Descriptor length
		db	DSCR_DEVICE	;; Decriptor type
		dw	0001H		;; Specification Version (BCD)
		db	00H  		;; Device class
		db	00H		;; Device sub-class
		db	00H		;; Device sub-sub-class
		db	64		;; Maximum packet size
		dw	5E09H		;; Vendor ID (095EH registered to USB Design Labs)
		dw	0101H		;; Product ID (0101H = USB IntelliKeys Firmware)
		dw	0100H		;; Product version ID
		db	1		;; Manufacturer string index
		db	2		;; Product string index
		db	0		;; Serial number string index
		db	1		;; Number of configurations
deviceDscrEnd:

ConfigDscr:	
		db	9		;; Descriptor length
		db	DSCR_CONFIG	;; Descriptor type
		db	ConfigLength	;; Configuration + End Points length (LSB)
		db	0		;; Config + End Points length (MSB)
$if JWH
		db	3		;; Number of interfaces
$else
		db	1
$endif
		db	1		;; Configuration number
		db	0		;; Configuration string

		;;
		;;  fwr 10-29-02
		;;  device is not capable of remote wakeup
		
		;;db	10100000b	;; Attributes (b7 - buspwr, b6 - selfpwr, b5 - rwu)
		db	10000000b	;; Attributes (b7 - buspwr, b6 - selfpwr, b5 - rwu)

		;;  fwr 11/28/00 final value is 100 mAmps.
		db	50  ;;db	250	;; Power requirement (x2 = required milliamps)

Interface0:
		db	9		;; Descriptor length
		db	DSCR_INTRFC	;; Descriptor type
		db	0		;; Zero-based index of this interface
		db	0		;; Alternate setting
		db	2		;; Number of end points 
		db	3		;; Interface class = HID
		db	0		;; Interface sub class
		db	0		;; Interface sub sub class
		db	0		;; Interface descriptor string index
HID0:
		db	9		;; Length
		db	21H		;; Type
		db	10H, 01H	;; HID class spec.# compliance (0110 = 1.1)
					;;  fwr 10-29-02 even though this says HID 1.1, 
					;;  we cover Win98 Gold by watching incoming 
					;;  control pipe data for commands
		db	0		;; Country localization (0=none)
		db	1		;; Number of class descriptors
		db	22H		;; Descriptor type to follow (report)
		db	Report0Length	;; Report descriptor length (LSB)
		db	0		;; Report descriptor length (MSB)
; Endpoint IN1
		db	7		;; Descriptor length
		db	DSCR_ENDPNT	;; Descriptor type
		db	10000001b	;; Endpoint number = IN1
  		db	ET_INT		;; Endpoint type
		db	64		;; Maximun packet size (LSB)
		db	0		;; Max packect size (MSB)
		db	4		;; Polling interval (milliseconds)
; Endpoint OUT2
		db	7		;; Descriptor length
		db	DSCR_ENDPNT	;; Descriptor type
		db	00000010b	;; Endpoint number = OUT2
		db	ET_INT		;; Endpoint type
		db	64		;; Maximun packet size (LSB)
		db	0		;; Max packect size (MSB)
		db	4		;; Polling interval (milliseconds)
;;
$if JWH
;; Declare the Keyboard Interface
Interface1:
		db	9		;; Descriptor length
		db	DSCR_INTRFC	;; Descriptor type
		db	1		;; Zero-based index of this interface
		db	0		;; Alternate setting
		db	2		;; Number of end points 
		db	3		;; Interface class = HID
		db	1 ;; 0		;; Interface sub class
		db	1 ;; 0		;; Interface sub sub class
		db	0		;; Interface descriptor string index
HID1:
		db	9		;; Length
		db	21H		;; Type
		db	10H, 01H	;; HID class spec. 1.1
		db	0		;; Country localization (0=none)
		db	1		;; Number of class descriptors
		db	22H		;; Descriptor type to follow (report)
		db	Report1Length	;; Report descriptor length (LSB)
		db	0		;; Report descriptor length (MSB)
; Endpoint IN3
		db	7		;; Descriptor length
		db	DSCR_ENDPNT	;; Descriptor type
		db	10000011b	;; Endpoint number = IN3
		db	ET_INT		;; Endpoint type
		db	8		;; Maximum packet size (LSB)
		db	0		;; Max packect size (MSB)
		db	4		;; Polling interval (milliseconds)
; Endpoint OUT3
		db	7		;; Descriptor length
		db	DSCR_ENDPNT	;; Descriptor type
		db	00000011b	;; Endpoint number = OUT3
  		db	ET_INT		;; Endpoint type
		db	8		;; Maximum packet size (LSB)
		db	0		;; Max packect size (MSB)
		db	10		;; Polling interval (milliseconds)
;;
;; Declare the Mouse Interface
Interface2:
		db	9		;; Descriptor length
		db	DSCR_INTRFC	;; Descriptor type
		db	2		;; Zero-based index of this interface
		db	0		;; Alternate setting
		db	1		;; Number of end points 
		db	3		;; Interface class = HID
		db	1 ;; 0		;; Interface sub class
		db	2 ;; 0		;; Interface sub sub class
		db	0		;; Interface descriptor string index
HID2:
		db	9		;; Length
		db	21H		;; Type
		db	10H, 01H	;; HID class spec.# compliance (0110 = 1.1)
		db	0		;; Country localization (0=none)
		db	1		;; Number of class descriptors
		db	22H		;; Descriptor type to follow (report)
		db	Report2Length	;; Report descriptor length (LSB)
		db	0		;; Report descriptor length (MSB)
; Endpoint IN4
		db	7		;; Descriptor length
		db	DSCR_ENDPNT	;; Descriptor type
		db	10000100b	;; Endpoint number = IN4
		db	ET_INT		;; Endpoint type
		db	64		;; Maximun packet size (LSB)
		db	0		;; Max packect size (MSB)
		db	4		;; Polling interval (milliseconds)
$endif

ConfigLength	EQU	$-ConfigDscr

;; Declare the custom report
Report0:
		db	06H, 0A0H, 0FFH	;; Usage Page (FFA0H = vendor defined)
		db	09H, 01H	;; Usage (Vendor defined)
		db	0A1H, 01H	;; Collection (Application)
;		db	09H, 02H	;; Usage (vendor defined)
;		db	0A1H, 00H	;; Collection (Physical)
;		db	06H, 0A1H, 0FFH	;; Usage Page (vendor defined)

;; The input report
		db	09H, 03H	;; Usage (vendor defined)
		db	09H, 04H	;; Usage (vendor defined)
		db	15H, 80H	;; Logical minimum (80H = -128)
		db	25H, 7FH	;; Logical maximum (7FH = 127)
;		db	35H, 00H	;; Physical minimum (0)
;		db	45H, 0FFH	;; Physical maximum (255)
		db	75H, 08H	;; Report size (8 bits)
		db	95H, 08H	;; Report count (8 fields)
		db	81H, 02H	;; Input (Data, Variable, Absolute)
;; The output report
		db	09H, 05H	;; Usage (vendor defined)
		db	09H, 06H	;; Usage (vendor defined)
;		db	15H, 80H	;; Logical minimum (80H = -128)
;		db	25H, 7FH	;; Logical maximum (7FH = 127)
;		db	35H, 00H	;; Physical minimum (0)
;		db	45H, 0FFH	;; Physical maximum (255)
;		db	75H, 08H	;; Report size (8 bits)
;		db	95H, 08H	;; Report count (8 fields)
		db	91H, 02H	;; Output (Data, Variable, Absolute)

;		db	0C0H		;; End Collection (Physical)
		db	0C0H		;; End Collection (Application)	
Report0Length	EQU	$-Report0


Report1:
$if JWH
; HID Keyboard Descriptor
	DB	5, 1			;  Usage_Page (Generic Desktop)
	DB	9, 6			;  Usage (Keyboard)
	DB	0A1H, 1			;  Collection (Application)
	DB	5, 7			;    Usage Page (Keycodes)
	DB	019H, 224		;      Usage_Minimum (224)
	DB	029H, 231		;      Usage_Maximum (231)
	DB	015H, 0			;      Logical_Minimum (0)
	DB	025H, 1			;      Logical_Maximum (1)
	DB	075H, 1			;      Report_Size (1)
	DB	095H, 8			;      Report_Count (8)
	DB	081H, 2			;      Input (Data,Var,Abs) = Modifier Byte
	DB	075H, 1			;      Report_Size (1)  
	DB	095H, 8			;      Report_Count (8)
	DB	081H, 1			;      Input (Constant) = Reserved Byte
	DB	019H, 0			;      Usage_Minimum (0)
	DB	029H, 101		;      Usage_Maximum (101)
	DB	075H, 8			;      Report_Size (8)
	DB	095H, 6			;      Report_Count (6)
	DB	081H, 0			;      Input (Data,Array) = Keycode Bytes(6)
	DB	5, 8			;    Usage Page (LEDs)
	DB	019H, 1			;      Usage_Minimum (1)
	DB	029H, 5			;      Usage_Maximum (5)
	DB	075H, 1			;      Report_Size (1)
	DB	095H, 5			;      Report_Count (5)
	DB	091H, 2			;      Output (Data,Var,Abs) = LEDs (5 bits)
	DB	095H, 3			;      Report_Count (3)
	DB	091H, 1			;      Output (Constant) = Pad (3 bits)
	DB	0C0H			;  End_Collection
$endif
Report1Length	EQU	$-Report1

Report2:
; HID Mouse descriptor
$if JWH
$if FRED_MOUSE_REPORT
	;;  made to be like Apple mouse
	DB	5, 1			;  Usage_Page (Generic Desktop)
	DB	9, 2			;  Usage (Mouse)
	DB	0A1H, 1			;  Collection (Application)
	DB	9, 1			;    Usage(Pointer)
	DB	0A1H, 0			;    Collection(Physical)
	DB	5, 9			;      Usage page (Buttons)
	DB	019H, 1			;        Usage_Minimum (Button1)
	DB	029H, 8			;        Usage_Maximum (Button8)
	DB	015H, 0			;        Logical_Minimum (0)
	DB	025H, 1			;        Logical_Maximum (1)
	DB	095H, 8			;        Report_Count (8)
	DB	075H, 1			;        Report_Size (1)
	DB	081H, 2			;        Input (Data,Var,Abs) = 4 Buttons
	DB	5, 1			;      Usage page (Generic desktop)
	DB	9, 30H			;        Usage (X)
	DB	9, 31H			;        Usage (Y)
	DB	015H, 81H		;        Logical_Minimum (-127)
	DB	025H, 7FH		;        Logical_Maximum (+127)
	DB	075H, 8			;        Report_Size (8)
	DB	095H, 2			;        Report_Count (2)
	DB	081H, 6			;        Input (Data, Var, Rel) = X, Y displacement
	DB	0C0H			;    End_Collection
	DB	0C0H			;  End_Collection
$else
	DB	5, 1			;  Usage_Page (Generic Desktop)
	DB	9, 2			;  Usage (Mouse)
	DB	0A1H, 1			;  Collection (Application)
	DB	9, 1			;    Usage(Pointer)
	DB	0A1H, 0			;    Collection(Physical)
	DB	5, 9			;      Usage page (Buttons)
	DB	019H, 1			;        Usage_Minimum (Button1)
	DB	029H, 4			;        Usage_Maximum (Button4)
	DB	015H, 0			;        Logical_Minimum (0)
	DB	025H, 1			;        Logical_Maximum (1)
	DB	075H, 1			;        Report_Size (1)
	DB	095H, 4			;        Report_Count (4)
	DB	081H, 2			;        Input (Data,Var,Abs) = 4 Buttons
	DB	081H, 3			;        Input (Constant, Abs) = Pad
	DB	5, 1			;      Usage page (Generic desktop)
	DB	9, 30H			;        Usage (X)
	DB	9, 31H			;        Usage (Y)
	DB	075H, 8			;        Report_Size (8)
	DB	095H, 2			;        Report_Count (2)
	DB	015H, 80H		;        Logical_Minimum (-128)
	DB	025H, 7FH		;        Logical_Maximum (+127)
	DB	081H, 6			;        Input (Data, Var, Rel) = X, Y displacement
	DB	0C0H			;    End_Collection
	DB	0C0H			;  End_Collection
$endif
$endif
Report2Length	EQU	$-Report2


;; Build a table that can be indexed
ReportDscr:	dw	Report0, Report1, Report2
ReportDscrLen:	db	Report0Length, Report1Length, Report2Length

HIDDscr:	dw	HID0, HID1, HID2

StringDscr:
StringDscr0:	
		db	StringDscr0End-StringDscr0		;; String descriptor length
		db	DSCR_STRING
		db	09H,04H
StringDscr0End:

StringDscr1:	
		db	StringDscr1End-StringDscr1		;; String descriptor length
		db	DSCR_STRING
		db	'I',0,'n',0,'t',0,'e',0,'l',0,'l',0,'i',0,'T',0,'o',0,'o',0,'l',0,'s',0
		db	',',0,' ',0,'I',0,'n',0,'c',0,'.',0
StringDscr1End:

StringDscr2:	
		db	StringDscr2End-StringDscr2		;; Descriptor length
		db	DSCR_STRING
		db	'I',0,'n',0,'t',0,'e',0,'l',0,'l',0,'i',0,'K',0,'e',0,'y',0,'s',0
		db	' ',0,'U',0,'S',0,'B',0
StringDscr2End:

UserDscr:		
		dw	0000H
		end
		
