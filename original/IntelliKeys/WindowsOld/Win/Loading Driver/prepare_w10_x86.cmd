@rem
@rem Copy driver, symbols, and INF to another directory to set up the cab file
@rem
mkdir w10_x86
copy Release\ikusb.sys w10_x86
copy Release\ikusb.pdb w10_x86
copy Release\ikusb.inf w10_x86
@rem
@rem Create the cab file
@rem
makecab /f w10_x86.ddf
@rem
@rem Sign the cab file
@rem 
SignTool sign /ac "GlobalSign Root CA.crt" /s MY /n "Echo Digital Audio Corporation" /fd sha256 /tr http://timestamp.globalsign.com/scripts/timestamp.dll  /td sha256 /v "disk1\ikusb_w10_x86.cab"
@pause