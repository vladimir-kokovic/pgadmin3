#ifndef __DLG_POLICYPROP
#define __DLG_POLICYPROP

#include "dlg/dlgProperty.h"

class pgPolicy;
class pgObject;

class dlgPolicy : public dlgProperty
{
public:
	dlgPolicy(pgaFactory *factory, frmMain *frame, pgPolicy *node = 0, pgObject *parentNode = 0);

	void CheckChange();
	wxString GetSql();
	pgObject *CreateObject(pgCollection *collection);
	pgObject *GetObject();
	wxString GetHelpPage() const
	{
		return wxT("pg/sql-altertable");
	}

	int Go(bool modal);

private:
	pgPolicy *policy;
	pgObject *object;

	void SetRolesToCtrl();
	void OnChangeValidate(wxCommandEvent &ev);

	void OnRoleSelChange(wxCommandEvent &ev);
	void OnAddRole(wxCommandEvent &ev);
	void OnDelRole(wxCommandEvent &ev);
	void OnRoleChange(wxCommandEvent &ev);

	DECLARE_EVENT_TABLE()
};

#endif
