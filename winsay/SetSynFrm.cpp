//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "SetSynFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
__fastcall TSettingsForm::TSettingsForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSettingsForm::Button1Click(TObject *Sender)
{
        Close();
}
//---------------------------------------------------------------------------
void __fastcall TSettingsForm::LangComboBoxChange(TObject *Sender)
{
        MainForm->LangChange(this);
}
//---------------------------------------------------------------------------
void __fastcall TSettingsForm::FormCreate(TObject *Sender)
{
        MainForm->LangRead(this);
}
//---------------------------------------------------------------------------
