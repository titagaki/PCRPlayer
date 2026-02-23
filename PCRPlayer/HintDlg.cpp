// HintDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "HintDlg.h"
//#include "afxdialogex.h"


// CHintDlg ダイアログ

IMPLEMENT_DYNAMIC(CHintDlg, CDialog)

CHintDlg::CHintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHintDlg::IDD, pParent)
	, align_(DT_CENTER)
{
	utl::createFont(font_, true);
}

CHintDlg::~CHintDlg()
{
	if (font_.GetSafeHandle()) { font_.DeleteObject(); }
}

void CHintDlg::SetConfig(const sl::HintConfig& cfg)
{
	hint_ = cfg;
	font_.DeleteObject();
	font_.CreateFontIndirect(&hint_.font);

	DestroyWindow();
	Create(IDD, GetDesktopWindow());

	if (!hint_.opaque.enable)
	{
		SetLayeredWindowAttributes(0, hint_.opaque.back, LWA_ALPHA);
	}
}


void CHintDlg::showWindow(bool show)
{
	if (show)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		// wndBottomやwndTopだとちらつきが発生する
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_HIDE);
	}
}


void CHintDlg::draw(CDC& dc, const CRect& org, bool mask, int text, int back)
{
	COLORREF color = hint_.text[0];
	if (mask)
	{
		color = RGB(text, text, text);
		dc.FillRect(org, &CBrush(RGB(back, back, back)));
	}
	else
	{
		utl::drawFrameRect(dc, &org, hint_.color);
	}

	CRect rc(org);
	rc.DeflateRect(hint_.marginX, hint_.marginY);

	dc.SetTextColor(color);
	dc.SetBkColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(text_.c_str(), -1, &rc, DT_NOPREFIX | align_);
}

void CHintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHintDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CHintDlg メッセージ ハンドラー

BOOL CHintDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	size_.SetSize(0, 0);

	if (!hint_.opaque.enable)
	{
		SetLayeredWindowAttributes(0, hint_.opaque.back, LWA_ALPHA);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CHintDlg::PreTranslateMessage(MSG* pMsg)
{
	// EnterとESCを押すと終了してしまうので処理をつぶす
    if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
        return FALSE;
    }
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CHintDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CHintDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: ここにメッセージ ハンドラー コードを追加します。
	// 描画メッセージで CDialog::OnPaint() を呼び出さないでください。

	CRect org;
	GetClientRect(&org);
	

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, org.Width(), org.Height());
	CBitmap* oldBmp = memDC.SelectObject(&bmp);
	CFont* oldFont = memDC.SelectObject(&font_);
	////////////////////////////////////////////////////////////////////////////////////

	draw(memDC, org);

	///////////////////////////////////////////////////////////////
	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}


void CHintDlg::setAlign(DWORD align)
{
	align_ = align;
}

void CHintDlg::setText(const std::wstring& text)
{
	text_ = text;

	if (text.empty())
	{
		size_.SetSize(0, 0);
		return;
	}

	CClientDC dc(this);
	CRect calc;
	CFont* fontOld = dc.SelectObject(&font_);
	dc.DrawText(text.c_str(), -1, &calc, DT_CALCRECT|DT_NOPREFIX|align_);
	size_.cx = calc.Width() + (hint_.marginX * 2);
	size_.cy = calc.Height() + (hint_.marginY * 2);

	if (hint_.opaque.enable)
	{
		CRect org(0, 0, size_.cx, size_.cy);

		utl::LayeredWindow layer(size_.cx, size_.cy, font_);

		if (layer)
		{
			draw(layer.getImageCDC(), org);
			draw(layer.getMaskCDC(), org, true, hint_.opaque.text, hint_.opaque.back);
			if (layer.create())
			{
				layer.update(GetSafeHwnd(), dc);
			}
		}
	}

	dc.SelectObject(fontOld);
}

void CHintDlg::setPosition(long x, long y)
{
	// 設置位置のモニタ情報取得
	MONITORINFO mi;
	CPoint pt(x, y);
	utl::getDesktopRectFromPoint(pt, mi);
	CRect rc(mi.rcMonitor);

	// モニターからはみ出さない位置を計算
	int ax = x - (size_.cx / 2), ay = y - size_.cy;

	if (x - (size_.cx / 2) < rc.left) { ax = rc.left; }// 左
	else if (x + (size_.cx / 2) > rc.right) { ax = rc.right - size_.cx; }// 右
	else { ax = x - (size_.cx / 2); }

	// 移動
	//MoveWindow(ax, ay, size_.cx, size_.cy);
	SetWindowPos(NULL, ax, ay, size_.cx, size_.cy, SWP_NOZORDER|SWP_NOACTIVATE);

}
