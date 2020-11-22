#include "HitDialog.h"
#include "resource.h"
#include "StateManager.h"
#include "pluginsdk/_plugins.h"

const char* hits = "Hits";
const char* instruction = "Instruction";
const char* oneStr = "1";

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		DestroyWindow(hwndDlg);
		return true;
	}
	break;
	case WM_INITDIALOG:
	{
		HWND listView = GetDlgItem(hwndDlg, IDC_HITS_TABLE);
		ListView_SetExtendedListViewStyle(listView, LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);

		LVCOLUMN column;
		memset(&column, 0, sizeof(column));
		column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		column.cx = 0xA8;
		column.cx = 0x40;

		column.pszText = (LPSTR)hits;
		ListView_InsertColumn(listView, 0, &column);

		column.pszText = (LPSTR)instruction;
		ListView_InsertColumn(listView, 1, &column);
		ListView_SetColumnWidth(listView, 1, LVSCW_AUTOSIZE_USEHEADER);
	}
	break;
	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMACTIVATE:
			if (((LPNMHDR)lParam)->idFrom == IDC_HITS_TABLE)
			{
				LRESULT id = ListView_GetSelectionMark(((LPNMHDR)lParam)->hwndFrom);

				char* info = StateManager::getInstance().getInfoByHwndAndIndex(hwndDlg, id);

				if (info != nullptr)
					SetWindowText(GetDlgItem(hwndDlg, IDC_CONTEXT_TEXT), info);
					//SendMessageA(((LPNMHDR)lParam)->hwndFrom, WM_SETTEXT, NULL, (LPARAM)info);

				return TRUE;
			}
			break;
		}
		break;
	}
	case WM_UPDATE_HITS:
	{
		int indexToUpdate = wParam;
		int hits = lParam;

		HWND listView = GetDlgItem(hwndDlg, IDC_HITS_TABLE);
		int rowCount = ListView_GetItemCount(listView);

		if (indexToUpdate >= rowCount)
			return false;

		char buffer[16];
		snprintf(buffer, sizeof(buffer), "%d", hits);
		ListView_SetItemText(listView, indexToUpdate, 0, buffer);
	}
	break;
	case WM_UPDATE_INSERT_ROW:
	{
		char* instruction = (char*)wParam;

		HWND listView = GetDlgItem(hwndDlg, IDC_HITS_TABLE);
		int rows = ListView_GetItemCount(listView);

		LVITEM item;
		item.iItem = rows;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.cchTextMax = 256;
		item.pszText = (LPSTR)oneStr;
		ListView_InsertItem(listView, &item);
		ListView_SetItemText(listView, rows, 1, instruction);
	}
	break;
	default:
		break;
	}

	return false;
}

void spawnDialog(void* userdata)
{
	HANDLE event = CreateEvent(NULL, TRUE, FALSE, TEXT("WaitForDialogEvent"));

	HitDialog* hitDialog = static_cast<HitDialog*>(userdata);

	HWND hwnd = CreateDialog(StateManager::getInstance().getHInstance(), MAKEINTRESOURCE(IDD_DIALOG_HITS), GuiGetWindowHandle(), DialogProc);
	hitDialog->setHWND(hwnd);

	char windowName[64];
	snprintf(windowName, sizeof(windowName), "Find out what accesses %X address", hitDialog->getAddress());
	SetWindowText(hwnd, windowName);
	ShowWindow(hwnd, SW_SHOW);

	SetEvent(event);
	CloseHandle(event);
}

HitDialog::HitDialog(duint address) :
	address(address)
{
	HANDLE event = CreateEvent(NULL, TRUE, FALSE, TEXT("WaitForDialogEvent"));
	GuiExecuteOnGuiThreadEx(spawnDialog, this);
	WaitForSingleObject(event, INFINITE);
	CloseHandle(event);
}

void HitDialog::setHWND(HWND hwnd)
{
	this->hwnd = hwnd;
}

HWND HitDialog::getHWND()
{
	return hwnd;
}

duint HitDialog::getAddress()
{
	return address;
}

void HitDialog::updateHits(int index, int hits)
{
	PostMessage(hwnd, WM_UPDATE_HITS, index, hits);
}

void HitDialog::insertRow(char* instruction)
{
	PostMessage(hwnd, WM_UPDATE_INSERT_ROW, (WPARAM)instruction, NULL);
}