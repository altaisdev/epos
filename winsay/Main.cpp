//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#define THIS_IS_A_TTSCP_CLIENT
#include "AboutFrm.h"
#include "SetSynFrm.h"
#include "Main.h"

#include "config.h"	/* You can usually remove this item */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define TTSCP_PORT	8778

const char *COMMENT_LINES = "#;\r\n";
const char *WHITESPACE = " \t\r";

const char *output_file = NULL;
//const char *output_file = "#localsound";

bool chunking = false;
bool show_segments = false;
bool wavfile = false;
bool wavstdout = false;

bool langset = false;
bool voiceset = false;

time_t t1, t2;
//struct diphone {
//	int code;
//	int f,e,t;
//};

struct segment {
	short code; char nothing; char ll;
	int f,e,t;
};

#define STDIN_BUFF_SIZE  550000

int ctrld, datad;		/* file descriptors for the control and data connections */
char *data = NULL;

char *ch;
char *dh;

void shriek(char *txt)
{
	fprintf(stderr, "Client side error: %s\n", txt);
//	exit(1);
}

void shriek(int, char *txt)
{
	shriek(txt);
}


#define EPOS_COMMON_H	// this is a lie
#include "exc.h"
#include "client.cpp"

int __fastcall TMainForm::get_result(int sd)
{
        bool kvo;
  	while (sgets(scratch, SCRATCH_SPACE, sd)) {
		scratch[SCRATCH_SPACE] = 0;
//		printf("Received: %s\n", scratch);
		switch(*scratch) {
			case '1': continue;
			case '2': return 2;
			case '3': break;
			case '4': printf("%s\n", scratch+strspn(scratch, "0123456789x "));
				  return 4;
			case '6': return 0; //if (!strncmp(scratch, "600 ", 4)) {
					//exit(0);
				  //} /* else fall through */
			case '8': printf("%s\n", scratch+strspn(scratch, "0123456789x "));
				  exit(2);

			case '5':
			case '7':
			case '9':
			case '0': printf("%s\n", scratch); shriek("Unhandled response code");
			default : ;
		}
		char *o = scratch+strspn(scratch, "0123456789 ");
		//if (*scratch && *o) printf("%s\n", o);
                if (*scratch && *o) switch(ResOut) {
		        case 0 : {/*SettingsForm->Dbg->Lines->Add(o);*/ break;}
                        case 1 : {
                                   if (!langset) {
                                     LangComboBox->Items->Add(o);
                                     LangComboBox->ItemIndex = 0;
                                   }
                                   break;
                                 }
                        case 2 : {
                                   if (!voiceset) {
                                     VoiceComboBox->Items->Add(o);
                                     VoiceComboBox->ItemIndex = 0;
                                   }
                                   break;
                                 }
                        default : ;
                }
	}
	return 8;	/* guessing */
}

int size;

char *get_data()
{
	char *b;
	size = 0;
	while (sgets(scratch, SCRATCH_SPACE, ctrld)) {
		scratch[SCRATCH_SPACE] = 0;
		if (strchr("2468", *scratch)) { 	/* all done, write result */
			if (*scratch != '2') shriek(scratch);
			if (!size) shriek("No processed data received");
			b[size] = 0;
			return b;
		}
		if (!strncmp(scratch, "123 ", 4)) {
			int count;
			sgets(scratch, SCRATCH_SPACE, ctrld);
			scratch[SCRATCH_SPACE] = 0;
			sscanf(scratch, "%d", &count);
			b = size ? (char *)realloc(b, size + count + 1) : (char *)malloc(count + 1);
			int limit = size + count;
			while (size < limit)
				size += yread(datad, b + size, limit - size);
		}
	}
	if (size) shriek("Disconnect during transmit");
	else shriek("Disconnect before transmit");
	return NULL;
}

void __fastcall TMainForm::say_data()
{
	if (!data) data = strdup("No.");
	sputs("strm $", ctrld);
	sputs(dh, ctrld);
	if (chunking) sputs(":chunk", ctrld);
	sputs(":raw:rules:diphs:synth:", ctrld);
	if (wavfile || wavstdout) sputs("$", ctrld), sputs(dh, ctrld);
	else sputs("#localsound", ctrld);
	sputs("\r\n", ctrld);
	sputs("appl ", ctrld);
	sprintf(scratch, "%d", (int)strlen(data));
	sputs(scratch, ctrld);
	sputs("\r\n", ctrld);
	sputs(data, datad);
	if (get_result(ctrld) > 2) shriek("Could not set up a stream");
	char *b;
	if (wavfile || wavstdout) {
		b = get_data();
		FILE *f = wavstdout ? stdout : fopen(output_file, "wb");
		if (!f || !fwrite(b, size, 1, f)) shriek("Could not write waveform");
		if (!wavstdout) fclose(f);
		free(b);
		return;
	}
	if (get_result(ctrld) > 2) shriek("Could not apply a stream");
}


void __fastcall TMainForm::trans_data()
{
	if (!data) data = "No.";
	sputs("strm $", ctrld);
	sputs(dh, ctrld);
	sputs(":raw:rules:print:$", ctrld);
	sputs(dh, ctrld);
	sputs("\r\n", ctrld);
	sputs("appl ", ctrld);
	sprintf(scratch, "%d", (int)strlen(data));
	sputs(scratch, ctrld);
	sputs("\r\n", ctrld);
	sputs(data, datad);
	get_result(ctrld);

	while (sgets(scratch, SCRATCH_SPACE, ctrld)) {
		scratch[SCRATCH_SPACE] = 0;
		if (*scratch > '1') return;	// guessing
		if (!strncmp(scratch, "122 ", 4)) {
			int count;
			sgets(scratch, SCRATCH_SPACE, ctrld);
			scratch[SCRATCH_SPACE] = 0;
			sscanf(scratch, "%d", &count);
			char *b=(char *)malloc(count+1);
			b[yread(datad, b, count)] = 0;
			DebugEdit->Lines->Add(b);
			//printf("%s\n", b);
		}
	}

	get_result(ctrld);
}

void __fastcall TMainForm::trans_diphones()
{
        char dphs[40];

	sputs("strm $", ctrld);
	sputs(dh, ctrld);
	sputs(":raw:rules:diphs:$", ctrld);
	sputs(dh, ctrld);
	sputs("\r\n", ctrld);
	sputs("appl ", ctrld);
	sprintf(scratch, "%d", (int)strlen(data));
	sputs(scratch, ctrld);
	sputs("\r\n", ctrld);
	sputs(data, datad);
	get_result(ctrld);

	while (sgets(scratch, SCRATCH_SPACE, ctrld)) {
		scratch[SCRATCH_SPACE] = 0;
		if (*scratch > '1') return;	// guessing
		if (!strncmp(scratch, "122 ", 4)) {
			int count;
			sgets(scratch, SCRATCH_SPACE, ctrld);
			scratch[SCRATCH_SPACE] = 0;
			sscanf(scratch, "%d", &count);
			segment *b=(segment *)malloc(count);
			yread(datad, b, count);
			for (int i=1;i<b[0].code;i++) {
                                sprintf(dphs,"%4.d - %3.d %3.d %3.d", b[i].code, b[i].f, b[i].e, b[i].t);
        			DiphonesEdit->Lines->Add(dphs);
                        }
		}
	}

	get_result(ctrld);
}

void __fastcall TMainForm::send_option(char *name, char *value)
{
	xmit_option(name, value, ctrld);
	get_result(ctrld);
}

char *get_relase(int sd)
{
	do {
		sgets(scratch, cfg->scratch, sd);
	} while (*scratch && strncmp(scratch, "release: ", 9));
	if (!*scratch) {
		printf("NULL handle\n");
		return NULL;
	}
	return strdup(scratch + 8);
}
/*
#define CMD_LINE_OPT "-"
#define CMD_LINE_VAL '='

void dump_help()
{
	printf("usage: say [options] ['text to be processed']\n");
	printf(" -b  bare format (no frills)\n");
	printf(" -c  casual pronunciation\n");
	printf(" -i  ironic pronunciation\n");
	printf(" -k  shutdown Epos\n");
	printf(" -l  list available languages and voices\n");
	printf(" -u  use utterance chunking\n");
	printf(" -w  write output to said.wav (in the \"pseudo root\" directory)\n");
}
			for (j=ar+1; *j; j++) switch (*j) {
				case 'b': send_option("out_verbose", "false"); break;
				case 'c': send_option("colloquial", "true"); break;
				case 'd': send_option("show_diphones", "true"); break;
				case 'H': send_option("long_help", "true");	// fall through
				case 'h': dump_help(); exit(0);
				case 'i': send_option("irony", "true"); break;
				case 'k': FILE *f;
					  f = fopen("/var/run/epos.pwd", "r");
					  if (!f) shriek("Cannot open /var/run/epos.pwd");
					  sputs("pass ", ctrld);
					  scratch[fread(scratch, 1, 1024, f)] = 0;
					  sputs(scratch, ctrld);
					  sputs("down\r\n", ctrld);
					  get_result(ctrld);
					  get_result(ctrld);
					  exit(0);
				case 'l': sputs("show languages\r\nshow voices\r\n", ctrld);
					  printf("Languages available:\n");
					  get_result(ctrld);
					  printf("Voices available for the current language:\n");
					  get_result(ctrld);
					  exit(0);
			//	case 'n': cfg->rules_file="nnet.rul";
			//		  if (this_lang)
			//			this_lang->rules_file = cfg->rules_file;
			//		  cfg->neuronet=true; break;
				case 'p': send_option("pausing", "true"); break;
				case 's': send_option("play_diph", "true"); break;
				case 'u': chunking = true; break;
				case 'v': send_option("version", "true"); break;
				case 'w': send_option("wave_header", "true");
					  output_file = "/said.wav"; break;
				case 'D':
					send_option("use_debug", "true");
			//		if (!cfg->use_dbg) cfg->use_dbg=true;
			//		else if (cfg->warnings)
			//			cfg->always_dbg--;
					break;
				default : shriek("Unknown short option");
			}
*/
#ifndef HAVE_STRDUP

char *strdup(const char*src)
{
	return strcpy((char *)malloc(strlen(src)+1), src);
}

#endif   // ifdef HAVE_STRDUP

#ifndef HAVE_TERMINATE

void terminate(void)
{
	abort();
}

#endif

//---------------------------------------------------------------------------
#pragma link "CSPIN"
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
#ifdef HAVE_WINSOCK2_H
	if (WSAStartup(MAKEWORD(2,0), (LPWSADATA)scratch)) shriek(464, "No winsock");
#endif
        ResOut = 0;
        Caption = "Winsay";
        FName = NULL;
        //ImageList1->FileLoad(rtIcon,"winsay.bmp",clSilver);
        NewButton->ImageIndex = 0;
        OpenButton->ImageIndex = 1;
        SaveButton->ImageIndex = 2;
        SpeakButton->ImageIndex = 3;
        SetButton->ImageIndex = 4;
        HelpButton->ImageIndex = 5;
        FileSaveItem->Enabled = False;
        FileSaveAsItem->Enabled = False;
        FilePrintItem->Enabled = False;
        HelpContentsItem->Enabled = False;
        HelpSearchItem->Enabled = False;
        HelpHowToUseItem->Enabled = False;
        SaveButton->Enabled = False;
        SpeakButton->Enabled = False;
        SetButton->Enabled = False;
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
        Application->OnHint = ShowHint;
        SetPanel->Visible = SetSynItem->Checked;
        SDPanel->Visible = ShowDebugItem->Checked;
        DiphonesPanel->Visible = ShowDiphonesItem->Checked;
        //eposfind();
//  if (ShowDebugItem->Checked) {
//    Memo1->Align = alLeft;
//    Memo1->Width = Width - 200;
//    Splitter1->Left = Width - 200;
//  } else {
//    Splitter1->Left = Width;
//    Memo1->Align = alClient;
//  }
//  Splitter1->Visible = ShowDebugItem->Checked;
//  DebugEdit->Visible = ShowDebugItem->Checked;
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::ShowHint(TObject *Sender)
{
	StatusLine->SimpleText = Application->Hint;
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FileNew(TObject *Sender)
{
        Memo1->Lines->Clear();
        Caption = "Winsay";
        FName = NULL;
        FileSaveItem->Enabled = False;
        FileSaveAsItem->Enabled = False;
        SaveWfmAsItem->Enabled = False;
        SaveButton->Enabled = False;
        SpeakButton->Enabled = False;
        FilePrintItem->Enabled = False;
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FileOpen(TObject *Sender)
{
	if (OpenDialog->Execute())
	{
                FName = OpenDialog->FileName;
		Memo1->Lines->LoadFromFile(OpenDialog->FileName);
                Caption = "Winsay - " + OpenDialog->FileName;
                //FileSaveItem->Enabled = True;
                FileSaveAsItem->Enabled = True;
                SaveWfmAsItem->Enabled = True;
                SpeakButton->Enabled = True;
                FilePrintItem->Enabled = True;
                //SaveButton->Enabled = True;
	}
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FileSave(TObject *Sender)
{
        if(FName != NULL) {
                Memo1->Lines->SaveToFile(FName);
                FileSaveItem->Enabled = False;
                FileSaveAsItem->Enabled = True;
                SaveWfmAsItem->Enabled = True;
                SaveButton->Enabled = False;
        }
        else FileSaveAs(this);
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FileSaveAs(TObject *Sender)
{
	if (SaveDialog->Execute())
	{
		Memo1->Lines->SaveToFile(SaveDialog->FileName);
                FileSaveItem->Enabled = False;
                FileSaveAsItem->Enabled = True;
                SaveWfmAsItem->Enabled = True;
                SaveButton->Enabled = False;
	}
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FilePrint(TObject *Sender)
{
	if (PrintDialog->Execute())
	{
		Print();
	}
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FilePrintSetup(TObject *Sender)
{
	PrintSetupDialog->Execute();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::FileExit(TObject *Sender)
{
	Close();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::EditUndo(TObject *Sender)
{
	Memo1->Undo();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::EditCut(TObject *Sender)
{
	Memo1->CutToClipboard();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::EditCopy(TObject *Sender)
{
	Memo1->CopyToClipboard();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::EditPaste(TObject *Sender)
{
	Memo1->PasteFromClipboard();
        if(!FileSaveItem->Enabled) {
                FileSaveItem->Enabled = True;
                FileSaveAsItem->Enabled = True;
                SaveWfmAsItem->Enabled = True;
                SaveButton->Enabled = True;
                SpeakButton->Enabled = True;
                FilePrintItem->Enabled = True;
        }
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpContents(TObject *Sender)
{
	Application->HelpCommand(HELP_CONTENTS, 0);
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpSearch(TObject *Sender)
{
	Application->HelpCommand(HELP_PARTIALKEY, Longint(""));
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpHowToUse(TObject *Sender)
{
	Application->HelpCommand(HELP_HELPONHELP, 0);
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::HelpAbout(TObject *Sender)
{
	FrmAbout->ShowModal();
}
//----------------------------------------------------------------------------
void __fastcall TMainForm::SpeakItemClick(TObject *Sender)
{
  if(Memo1->Text.Length()) {
        DebugEdit->Lines->Clear();
        DiphonesEdit->Lines->Clear();
	ctrld = connect_socket(0, TTSCP_PORT);
	ch = get_handle(ctrld);
	datad = connect_socket(0, TTSCP_PORT);
	sputs("data ", datad);
	sputs(ch, datad);
	sputs("\r\n", datad);
	free(ch);
	dh = get_handle(datad);
	get_result(datad);
        send_option("language", LangComboBox->Items->Strings[LangComboBox->ItemIndex].c_str());
        send_option("voice", VoiceComboBox->Items->Strings[VoiceComboBox->ItemIndex].c_str());
        send_option("wave_header", "true");
        send_option("show_diphones", "true");
        if (UseProsodyBox->Checked) {
                send_option("init_f", IntToStr(PitchEdit->Value).c_str());
                send_option("init_i", IntToStr(IntEdit->Value).c_str());
                send_option("init_t", IntToStr(RateEdit->Value).c_str());
        }
        data = strdup(Memo1->Lines->Text.c_str());
        if (!UseProsodyRulBox->Checked) send_option("use_prosody", "false");
        else send_option("use_prosody", "true");
        if (TSRBox->Checked) send_option("tsr_tool", "true");
        //if (LPCProsBox->Checked) send_option("lpc_prosody", "true");
        //else send_option("lpc_prosody", "false");
        t1 = time(NULL);
	say_data();
        t2 = time(NULL);
        TimeEdit->Text = IntToStr(t2-t1);
        if (TSRBox->Checked) send_option("tsr_tool", "false");
	trans_data();
	trans_diphones();
//	printf("\n");
	sputs("delh ", ctrld);
	sputs(dh, ctrld);
	sputs("\r\ndone\r\n", ctrld);
	get_result(ctrld);
	get_result(ctrld);
	close(datad);
	close(ctrld);
        wavfile = false;
        //DebugEdit->Lines->Add("-----------------------------------------------------------------------");
        //DiphonesEdit->Lines->Add("-------------------");
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Memo1KeyPress(TObject *Sender, char &Key)
{
        if(!FileSaveItem->Enabled) {
                FileSaveItem->Enabled = True;
                FileSaveAsItem->Enabled = True;
                SaveWfmAsItem->Enabled = True;
                SaveButton->Enabled = True;
                SpeakButton->Enabled = True;
                FilePrintItem->Enabled = True;
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SetSynItemClick(TObject *Sender)
{
        SetSynItem->Checked = !SetSynItem->Checked;
        SetPanel->Visible = SetSynItem->Checked;
        if (SetPanel->Visible && !SDPanel->Visible) {
                SDPanel->Height = SetPanel->Height;
                SDPanel->Visible = true;
        }
        if (!SetPanel->Visible && !ShowDebugItem->Checked)
                SDPanel->Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::eposfind(void)
{
	ctrld = connect_socket(0, TTSCP_PORT);
        if (ctrld != -1) {
        	ch = get_relase(ctrld);
                StatusLine->SimpleText = "Connected to Epos" + AnsiString(ch);
        	free(ch);
                close(ctrld);

        	ctrld = connect_socket(0, TTSCP_PORT);
        	ch = get_handle(ctrld);
        	datad = connect_socket(0, TTSCP_PORT);
        	sputs("data ", datad);
        	sputs(ch, datad);
        	sputs("\r\n", datad);
        	free(ch);
        	dh = get_handle(datad);
        	get_result(datad);

                sputs("show languages\r\nshow voices\r\n", ctrld);
        	//SettingsForm->Dbg->Lines->Add("Languages available:\n");
                ResOut = 1;
        	get_result(ctrld);
        	//SettingsForm->Dbg->Lines->Add("Voices available for the current language:\n");
                ResOut = 2;
        	get_result(ctrld);
                ResOut = 0;
        	sputs("\r\ndone\r\n", ctrld);
        //	get_result(ctrld);
        //	get_result(ctrld);
        	close(datad);
        	close(ctrld);
                SetButton->Enabled = True;
                voiceset = true;
        } else {
                StatusLine->SimpleText = "No connected";
                //Application->MessageBox("Error 473: Epos server unreachable...",NULL, MB_OK);
                SetButton->Enabled = False;
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LangChange(TObject *Sender)
{
  langset = true; voiceset = false;
	ctrld = connect_socket(0, TTSCP_PORT);
	ch = get_handle(ctrld);
	datad = connect_socket(0, TTSCP_PORT);
	sputs("data ", datad);
	sputs(ch, datad);
	sputs("\r\n", datad);
	free(ch);
	dh = get_handle(datad);
	get_result(datad);
        send_option("language",LangComboBox->Items->Strings[LangComboBox->ItemIndex].c_str());
        VoiceComboBox->Items->Clear();
        sputs("show voices\r\n", ctrld);
        ResOut = 2;
	get_result(ctrld);
        ResOut = 0;
	sputs("\r\ndone\r\n", ctrld);
	close(datad);
	close(ctrld);
  Timer1->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
	ctrld = connect_socket(0, TTSCP_PORT);
        if (ctrld != -1) {
        	ch = get_relase(ctrld);
                StatusLine->SimpleText = "Connected to Epos" + AnsiString(ch);
        	free(ch);
                close(ctrld);

        	ctrld = connect_socket(0, TTSCP_PORT);
        	ch = get_handle(ctrld);
        	datad = connect_socket(0, TTSCP_PORT);
        	sputs("data ", datad);
        	sputs(ch, datad);
        	sputs("\r\n", datad);
        	free(ch);
        	dh = get_handle(datad);
        	get_result(datad);

                sputs("show languages\r\nshow voices\r\n", ctrld);
        	//SettingsForm->Dbg->Lines->Add("Languages available:\n");
                ResOut = 1;
        	get_result(ctrld);
        	//SettingsForm->Dbg->Lines->Add("Voices available for the current language:\n");
                ResOut = 2;
        	get_result(ctrld);
                ResOut = 0;
        	sputs("\r\ndone\r\n", ctrld);
        //	get_result(ctrld);
        //	get_result(ctrld);
        	close(datad);
        	close(ctrld);
                SetButton->Enabled = True;
                voiceset = true;
        } else {
                SetButton->Enabled = False;
                StatusLine->SimpleText = "No connected";
        }
        Timer1->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PitchBarChange(TObject *Sender)
{
  PitchEdit->Value = PitchBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PitchEditChange(TObject *Sender)
{
  PitchBar->Position = PitchEdit->Value;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RateBarChange(TObject *Sender)
{
  RateEdit->Value = RateBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RateEditChange(TObject *Sender)
{
  RateBar->Position = RateEdit->Value;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IntBarChange(TObject *Sender)
{
  IntEdit->Value = IntBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IntEditChange(TObject *Sender)
{
  IntBar->Position = IntEdit->Value;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShowDebugItemClick(TObject *Sender)
{
  ShowDebugItem->Checked = !ShowDebugItem->Checked;
  SDPanel->Visible = ShowDebugItem->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShowDiphonesItemClick(TObject *Sender)
{
  ShowDiphonesItem->Checked = !ShowDiphonesItem->Checked;
  DiphonesPanel->Visible = ShowDiphonesItem->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormActivate(TObject *Sender)
{
  //eposfind();
  Timer1->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LangComboBoxChange(TObject *Sender)
{
  langset = true; voiceset = false;
  VoiceComboBox->Items->Clear();
  Timer1->Enabled = true;
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::SaveWfmAsItemClick(TObject *Sender)
{
  SaveWfmDialog->FileName = "said.wav";
  if (SaveWfmDialog->Execute()) {
        //send_option("wave_header", "true");
	wavfile = true;
	output_file = SaveWfmDialog->FileName.c_str();
        SpeakItemClick(this);
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Memo1Change(TObject *Sender)
{
        if(!FileSaveItem->Enabled) {
                FileSaveItem->Enabled = True;
                FileSaveAsItem->Enabled = True;
                SaveWfmAsItem->Enabled = True;
                SaveButton->Enabled = True;
                SpeakButton->Enabled = True;
                FilePrintItem->Enabled = True;
        }
}
//---------------------------------------------------------------------------


