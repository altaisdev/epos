//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "AboutFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFrmAbout *FrmAbout;
//---------------------------------------------------------------------------
__fastcall TFrmAbout::TFrmAbout(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFrmAbout::Button1Click(TObject *Sender)
{
        Close();        
}
//---------------------------------------------------------------------------

