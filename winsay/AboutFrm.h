//---------------------------------------------------------------------------
#ifndef AboutFrmH
#define AboutFrmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TFrmAbout : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TButton *Button1;
        void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TFrmAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFrmAbout *FrmAbout;
//---------------------------------------------------------------------------
#endif
