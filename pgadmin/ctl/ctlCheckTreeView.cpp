//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// ctlCheckTreeView.cpp - TreeView with Checkboxes
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/imaglist.h>
#include <wx/wizard.h>
#include <wx/treectrl.h>

// App headers
#include "ctl/ctlCheckTreeView.h"
#include "images/checked.pngc"
#include "images/disabled.pngc"
#include "images/unchecked.pngc"
#include "utils/pasteTables.h"

BEGIN_EVENT_TABLE(ctlCheckTreeView, wxTreeCtrl)
	EVT_LEFT_DOWN(                            ctlCheckTreeView::OnLeftClick)
END_EVENT_TABLE()


ctlCheckTreeView::ctlCheckTreeView(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
	: wxTreeCtrl(parent, id, pos, size, style), copytables(0)
{
	wxImageList *treeimages = new wxImageList(16, 16, true, 3);
	treeimages->Add(*unchecked_png_img);
	treeimages->Add(*checked_png_img);
	treeimages->Add(*disabled_png_img);
	SetImageList(treeimages);
}


void ctlCheckTreeView::OnLeftClick(wxMouseEvent &evt)
{
	int flags;
	wxTreeItemId node = HitTest(evt.GetPosition(), flags);
	int newimage = 0;

	if (copytables)
	{
		if ((flags & wxTREE_HITTEST_ONITEMLABEL) || (flags & wxTREE_HITTEST_ONITEMICON))
		{
			if (GetItemImage(node) == 0)
				newimage = 1;
			else if (GetItemImage(node) == 1)
				newimage = 0;
			int level = 0;
			wxTreeItemId parent = GetItemParent(node);
			while (parent)
			{
				level++;
				parent = GetItemParent(parent);
			}
			//level == 0 root(Server Groups)
			//level == 1 group
			//level == 2 server
			//level == 3 db
			//level == 4 schema
			//level == 5 table
			switch (level)
			{
				case 4:
				{
					wxTreeItemIdValue childData;
					wxTreeItemId child = GetFirstChild(node, childData);
					while (child)
					{
						SetItemImage(child, newimage);
						child = GetNextChild(node, childData);
					}
					SetItemImage(node, newimage);
					break;
				}
				case 5:
					SetItemImage(node, newimage);
					break;
				default:
					break;
			}
			if (newimage == 1 && level < 5 && !HasChildren(node))
			{
				wxTreeItemId servernode, dbnode, schemanode;
				int level1 = level + 1;
				wxString servername, dbname, schemaname;
				parent = node;
				while (parent)
				{
					level1--;
					switch (level1)
					{
						case 2:
							servername = GetItemText(parent);
							servernode = parent;
							break;
						case 3:
							dbname = GetItemText(parent);
							dbnode = parent;
							break;
						case 4:
							schemaname = GetItemText(parent);
							schemanode = parent;
							break;
						default:
							break;
					}
					parent = GetItemParent(parent);
				}
				frmCopyTables *frm = (frmCopyTables *)copytables;
				bool success = frm->treeDetails(servername, dbname, schemaname);
				if (success)
				{
					SetItemImage((dbname.IsEmpty()) ? servernode : (schemaname.IsEmpty()) ? dbnode : schemanode, 0);
					Expand((dbname.IsEmpty()) ? servernode : (schemaname.IsEmpty()) ? dbnode : schemanode);
				}
			}
		}
	}
	else
	{
        if ((flags & wxTREE_HITTEST_ONITEMLABEL) || (flags & wxTREE_HITTEST_ONITEMICON))
        {
            if (GetItemImage(node) == 0)
                newimage = 1;
            else if (GetItemImage(node) == 1)
                newimage = 0;

            if (newimage == 0 || newimage == 1)
                SetParentAndChildImage(node, newimage);
            if (newimage == 1)
                SetParentImage(node, newimage);
        }
	}

	evt.Skip();
}

void ctlCheckTreeView::SetParentAndChildImage(wxTreeItemId node, int newimage)
{
	SetItemImage(node, newimage);
	wxTreeItemIdValue childData;
	wxTreeItemId child = GetFirstChild(node, childData);
	while (child.IsOk())
	{
		SetParentAndChildImage(child, newimage);
		child = GetNextChild(node, childData);
	}
}

void ctlCheckTreeView::SetParentImage(wxTreeItemId node, int newimage)
{
	if (node.IsOk())
	{
		SetItemImage(node, newimage);
		SetParentImage(GetItemParent(node), newimage);
	}
}

bool ctlCheckTreeView::IsChecked(const wxTreeItemId &node)
{
	return (GetItemImage(node) == 1);
}

