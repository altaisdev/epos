//---------------------------------------------------------------------------
#ifndef SetSynFrmH
#define SetSynFrmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TSettingsForm : public TForm
{
__published:	// IDE-managed Components
        TButton *Button1;
        TComboBox *LangComboBox;
        TComboBox *VoiceComboBox;
        TMemo *Dbg;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall LangComboBoxChange(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSettingsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
#endif
