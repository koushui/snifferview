#include "StreamDlg.h"
#include "../resource.h"
#include "../common/winsize.h"
#include "../common/common.h"
#include "../common/StrUtil.h"
#include "../global.h"
#include "../SyntaxHlpr/SyntaxDef.h"
#include "../FileCache/FileCache.h"

using namespace std;

enum StreamShowType {
    em_stream_utf8 = 0,
    em_stream_gbk,
    em_stream_unicode,
    em_stream_hex
};

map<HWND, CStreamDlg *> CStreamDlg::msPtrMap;

CStreamDlg::CStreamDlg(int pos) {
    mCurPos = pos;
    mLineMax = 128;
    mCurSel = -1;
}

CStreamDlg::~CStreamDlg() {
}

bool CStreamDlg::Create(HWND parent) {
    mParent = parent;
    CreateDialogParamA(g_m, MAKEINTRESOURCEA(IDD_STREAM), parent, DlgProc, (LPARAM)this);
    return true;
}

bool CStreamDlg::DoModule(HWND parent) {
    mParent = parent;
    DialogBoxParamA(NULL, MAKEINTRESOURCEA(IDD_STREAM), parent, DlgProc, (LPARAM)this);
    return true;
}

void CStreamDlg::InitSyntaxTextView() {
}

/*Window Message*/
void CStreamDlg::OnInitDlg(WPARAM wp, LPARAM lp) {
    SendMessageA(mhWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)LoadIconA(g_m, MAKEINTRESOURCEA(IDI_MAIN)));
    mFind = GetDlgItem(mhWnd, IDC_STREAM_FIND);
    mShowSelect = GetDlgItem(mhWnd, IDC_STREAM_SELECT);

    GetPacketSet();
    SendMessageA(mShowSelect, CB_INSERTSTRING, em_stream_utf8, (LPARAM)"utf-8");
    SendMessageA(mShowSelect, CB_INSERTSTRING, em_stream_gbk, (LPARAM)"gbk");
    SendMessageA(mShowSelect, CB_INSERTSTRING, em_stream_unicode, (LPARAM)"unicode");
    SendMessageA(mShowSelect, CB_INSERTSTRING, em_stream_hex, (LPARAM)"hex");
    SendMessageA(mShowSelect, CB_SETCURSEL, 0, 0);

    RECT clientRect = {0};
    GetClientRect(mhWnd, &clientRect);
    RECT rt1 = {0};
    GetWindowRect(mFind, &rt1);
    MapWindowPoints(NULL, mhWnd, (LPPOINT)&rt1, 2);

    int clientWidth = clientRect.right - clientRect.left;
    int clientHigh = clientRect.bottom - clientRect.top;

    int top = rt1.bottom + 8;
    int bottom = clientRect.bottom - 16;
    mShowView.InitStreamView(mhWnd, rt1.left, rt1.bottom + 8, clientWidth - (rt1.left * 2), bottom - top);
    HWND hShowView = mShowView.GetWindow();
    CTL_PARAMS arry[] = {
        {0, mFind, 0, 0, 1, 0},
        {0, mShowSelect, 1, 0, 0, 0},
        {0, hShowView, 0, 0, 1, 1}
    };
    SetCtlsCoord(mhWnd, arry, RTL_NUMBER_OF(arry));

    int cw = GetSystemMetrics(SM_CXSCREEN);
    int ch = GetSystemMetrics(SM_CYSCREEN);
    int cx = (cw / 4 * 3);
    int cy = (ch / 4 * 3);
    SetWindowPos(mhWnd, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE);
    CenterWindow(NULL, mhWnd);

    RECT wndRect = {0};
    GetWindowRect(mhWnd, &wndRect);
    //SetWindowRange(mhWnd, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, 0, 0);
}

void CStreamDlg::OnCommand(WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);
    if (id == IDC_STREAM_SELECT)
    {
        int curSel = SendMessageA(mShowSelect, CB_GETCURSEL, 0, 0);
        if (mCurSel == curSel)
        {
            return;
        }

        mCurSel = curSel;
        switch (curSel) {
            case em_stream_gbk:
            case em_stream_utf8:
            case em_stream_unicode:
                {
                    //mShowView.ClearView();
                    //LoadPacketSet(curSel);
                }
                break;
            case em_stream_hex:
                {
                    //mShowView.ClearView();
                    //LoadPacketSet(curSel);
                }
                break;
        }
    }
}

void CStreamDlg::OnClose(WPARAM wp, LPARAM lp) {

}

void CStreamDlg::OnMessage(UINT msg, WPARAM wp, LPARAM lp) {
}

INT_PTR CStreamDlg::DlgProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    CStreamDlg *ptr = NULL;
    map<HWND, CStreamDlg *>::const_iterator it = msPtrMap.find(hdlg);
    if (it != msPtrMap.end())
    {
        ptr = it->second;
        ptr->OnMessage(msg, wp, lp);
    }

    switch(msg)
    {
    case  WM_INITDIALOG:
        {
            msPtrMap[hdlg] = (CStreamDlg *)lp;
            ptr = (CStreamDlg *)lp;
            ptr->mhWnd = hdlg;
            ptr->OnInitDlg(wp, lp);
        }
        break;
    case WM_COMMAND:
        {
            ptr->OnCommand(wp, lp);
        }
        break;
    case  WM_CLOSE:
        {
            ptr->OnClose(wp, lp);
            EndDialog(hdlg, 0);
            msPtrMap.erase(hdlg);
        }
        break;
    default:
        break;
    }
    return 0;
}

void CStreamDlg::GetPacketSet() {
    if (CFileCache::GetInst()->GetShowCount() <= (size_t)mCurPos)
    {
        return;
    }

    PacketContent content;
    CFileCache::GetInst()->GetShow(mCurPos, content);

    mstring unique = content.m_packet_mark;
    mUnique1 = content.m_dec_mark;

    for (size_t i = 0 ; i < CFileCache::GetInst()->GetPacketCount() ; i++)
    {
        PacketContent tmp;
        CFileCache::GetInst()->GetPacket(i, tmp);
        if (tmp.m_packet_mark != unique)
        {
            continue;
        }

        PacketContent *ss = new PacketContent;
        *ss = tmp;
        mPacketSet.push_back(ss);

        if (mUnique2.empty() && tmp.m_dec_mark != mUnique1)
        {
            mUnique2 = tmp.m_dec_mark;
        }
    }
}

void CStreamDlg::AddData(const mstring &desc, const mstring &data) {
    size_t lastPos = 0;
    for (size_t i = 0 ; i < data.size() ; i++)
    {
        mstring lineStr;
        if (i - lastPos >= (size_t)mLineMax)
        {
            lineStr = data.substr(lastPos, i - lastPos);
            mShowView.AppendText(desc, lineStr);
            mShowView.AppendText(desc, "\n");
            lastPos = i;
            continue;
        }

        char c = data[i];
        if (c == '\n')
        {
            if (i >= lastPos)
            {
                lineStr = data.substr(lastPos, i - lastPos + 1);
                mShowView.AppendText(desc, lineStr);
                lastPos = i + 1;
            }
        }
    }
}

void CStreamDlg::PrintHex(const mstring &desc, const mstring &content) {
    mstring showData;
    for (size_t i = 0 ; i < content.size() ; i += 16)
    {
        DWORD dwReadSize = 0;
        if (content.size() < (i + 16))
        {
            dwReadSize = (content.size() - i);
        }
        else
        {
            dwReadSize = 16;
        }

        mstring curLine = content.substr(i, dwReadSize);
        showData += FormatA("%08x  ", i);
        size_t j = 0;
        for (j = 0 ; j < 16 ; j++)
        {
            if (j < dwReadSize)
            {
                showData += FormatA("%02x ", (BYTE)curLine[j]);
            }
            else
            {
                showData += "   ";
            }
        }

        showData += " ";
        showData += GetPrintStr(curLine.c_str(), curLine.size(), false);
        showData += "\n";
    }
    mShowView.AppendText(desc, showData);
    mShowView.AppendText(desc, "\n");
}

void CStreamDlg::LoadPacketSet(int type) {
    mstring lastUnique;
    for (list<PPacketContent>::const_iterator it = mPacketSet.begin() ; it != mPacketSet.end() ; it++)
    {
        PacketContent *ptr = *it;
        size_t offset = 0;
        if (ptr->m_tls_type == em_tls_tcp)
        {
            offset = sizeof(IPHeader) + ptr->m_tls_header.m_tcp.get_tcp_header_length();
        }
        else if (ptr->m_tls_type == em_tls_udp)
        {
        }

        mstring desc;
        if (ptr->m_dec_mark == mUnique1)
        {
            desc = LABEL_TCP_PIPE1;
        } else {
            desc = LABEL_TCP_PIPE2;
        }

        mstring showData = ptr->m_packet.substr(offset, ptr->m_packet.size() - offset);
        if (type == em_stream_hex)
        {
            PrintHex(desc, showData);
        } else {
            if (type == em_stream_utf8)
            {
                showData = UtoA(showData);
            } else if (type == em_stream_unicode)
            {
                showData = WtoA(wstring((LPCWSTR)showData.c_str(), showData.size() / 2));
            }

            if (lastUnique.empty())
            {
                lastUnique = ptr->m_dec_mark;
            } else if (lastUnique != ptr->m_dec_mark) {
                lastUnique = ptr->m_dec_mark;
                mShowView.AppendText(desc, "\n");
            }

            mstring show1 = GetPrintStr(showData.c_str(), showData.size());
            AddData(desc, show1);
        }
    }
}

void ShowStreamView(HWND hParent, int curPos) {
    CStreamDlg *ptr = new CStreamDlg(curPos);
    ptr->Create(hParent);
}