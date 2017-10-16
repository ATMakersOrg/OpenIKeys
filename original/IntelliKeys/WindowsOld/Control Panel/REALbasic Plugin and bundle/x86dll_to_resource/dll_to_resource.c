static void Initialize(void)
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
}
void main(void)
{
	StandardFileReply sfr;
	Initialize();
	StandardGetFile(nil, 1, (OSType *) "iDLL", &sfr);
	if (sfr.sfGood)
	{
		short ph, rph;
		Handle h;
		long len;
		FSSpec resourceSpec;
		int pos;
		int nameLen;
		if (noErr != FSpOpenDF(&sfr.sfFile, fsRdPerm, &ph))
			return;
		GetEOF(ph, &len);
		nameLen = sfr.sfFile.name[0];
		h = NewHandleClear(4 + 1 + nameLen + len);
		if (!h)
		{
			FSClose(ph);
			return;
		}
		HLock(h);
		pos = 4;
		BlockMoveData(sfr.sfFile.name, (*h) + pos, 1 + nameLen);
		pos += 1 + nameLen;
		FSRead(ph, &len, (*h) + pos);
		FSClose(ph);
		FSMakeFSSpec(sfr.sfFile.vRefNum, sfr.sfFile.parID, "\pDLL resource", &resourceSpec);
		FSpDelete(&resourceSpec);
		FSpCreateResFile(&resourceSpec, 'RSED', 'rsrc', smSystemScript);
		if (ResError() != noErr)
			return;
		rph = FSpOpenResFile(&resourceSpec, fsRdWrPerm);
		if (rph == -1)
			return;
		AddResource(h, 'PL86', 128, "\p");
		CloseResFile(rph);
	}
}
