#include "CPicker.h"
#include <algorithm>
#include <stdio.h>
#include <string>
#include <wingdi.h>
#include <WinUser.h>
#include <CommCtrl.h>

#include "resource.h"

int CALLBACK ColourPickerDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PickerData *picker_data = reinterpret_cast<PickerData *>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
	if (!picker_data && uMsg != WM_INITDIALOG)
	{
		return 0;
	}

	const HWND Color1 = GetDlgItem(hDlg, IDC_COLOR);
	const HWND Color2 = GetDlgItem(hDlg, IDC_COLOR2);
	const HWND Alpha = GetDlgItem(hDlg, IDC_ALPHASLIDE);

	RECT rectC1;
	RECT rectC2;
	RECT rectA;

	GetWindowRect(Color1, &rectC1);
	GetWindowRect(Color2, &rectC2);
	GetWindowRect(Alpha, &rectA);

	const float widthC1 = rectC1.right - rectC1.left;
	const float heightC1 = rectC1.bottom - rectC1.top;

	const float widthC2 = rectC2.right - rectC2.left;
	const float heightC2 = rectC2.bottom - rectC2.top;

	const float widthA = rectA.right - rectA.left;
	const float heightA = rectA.bottom - rectA.top;

	BOOL result;
	const int red = SendDlgItemMessage(hDlg, IDC_RSLIDER, UDM_GETPOS, 0, (LPARAM)&result);
	const int green = SendDlgItemMessage(hDlg, IDC_GSLIDER, UDM_GETPOS, 0, (LPARAM)&result);
	const int blue = SendDlgItemMessage(hDlg, IDC_BSLIDER, UDM_GETPOS, 0, (LPARAM)&result);

	const int hue = SendDlgItemMessage(hDlg, IDC_HSLIDER, UDM_GETPOS, 0, (LPARAM)&result);
	const int saturation = SendDlgItemMessage(hDlg, IDC_SSLIDER, UDM_GETPOS, 0, (LPARAM)&result);
	const int value = SendDlgItemMessage(hDlg, IDC_VSLIDER, UDM_GETPOS, 0, (LPARAM)&result);

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		picker_data = reinterpret_cast<PickerData *>(lParam);

		CColourPicker::PickerMap[picker_data->picker->Value] = hDlg;

		for (const std::pair<const unsigned int, std::pair<unsigned int, unsigned int>> &slider_combo : SLIDER_MAP)
		{
			SendDlgItemMessage(hDlg, slider_combo.second.first, UDM_SETBUDDY, (WPARAM)GetDlgItem(hDlg, slider_combo.first), 0);
			SendDlgItemMessage(hDlg, slider_combo.second.first, UDM_SETRANGE32, 0, slider_combo.second.second);
		}
		SendDlgItemMessage(hDlg, IDC_HEXSLIDER, UDM_SETBASE, 16, 0);
		SendDlgItemMessage(hDlg, IDC_HEXCOL, EM_SETLIMITTEXT, 10, 0);

		UpdateValues(hDlg, picker_data->picker->GetCurrentColour(), picker_data->changing_text);

		SendDlgItemMessage(hDlg, IDC_R, BM_SETCHECK, BST_CHECKED, 0);

		[[clang::fallthrough]];
	}

	case WM_DPICHANGED:
	{
		picker_data->bufferC1.Destroy();
		picker_data->bufferC2.Destroy();
		picker_data->bufferA.Destroy();
		picker_data->bufferC1.Create(widthC1, heightC1);
		picker_data->bufferC2.Create(widthC2, heightC2);
		picker_data->bufferA.Create(widthA, heightA);
		RedrawWindow(hDlg, NULL, NULL, RDW_UPDATENOW | RDW_INTERNALPAINT);

		break;
	}
	case WM_PAINT:
	{
		const HWND CurrentColor = GetDlgItem(hDlg, IDC_CURRCOLOR);
		const HWND OldColor = GetDlgItem(hDlg, IDC_OLDCOLOR);

		HDC hdc, hcomp;
		HBITMAP hbmp;
		const HBRUSH backgroundColorBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
		LOGBRUSH backgroundColor;
		GetObject(backgroundColorBrush, sizeof(backgroundColor), &backgroundColor);
		float rf, gf, bf;

		// Big color selector (displays non-selected features)
		// For example, if G is selected (in RGB radio box) it displays red vs. blue colors
		hdc = GetDC(Color1);
		hcomp = CreateCompatibleDC(hdc);

		hbmp = CreateCompatibleBitmap(hdc, widthC1, heightC1);
		SelectObject(hcomp, hbmp);

		// Check who is selected.
		// RED
		if (IsDlgButtonChecked(hDlg, IDC_R) == BST_CHECKED)
		{
			for (int g = 0; g < widthC1; g++)
			{
				gf = (g / widthC1) * 255.0f;
				for (int b = 0; b < heightC1; b++)
				{
					bf = (b / heightC1) * 255.0f;
					picker_data->bufferC1.SetPixel(g, b, RGB(red, gf, bf));
				}
			}

			picker_data->bufferC1.Display(hcomp);
			DrawCircle(hcomp, red, green, blue, (green / 255.0f) * widthC1, (blue / 255.0f) * heightC1);
		}

		// GREEN
		else if (IsDlgButtonChecked(hDlg, IDC_G) == BST_CHECKED)
		{
			for (int r = 0; r < widthC1; r++)
			{
				rf = (r / widthC1) * 255.0f;
				for (int b = 0; b < heightC1; b++)
				{
					bf = (b / heightC1) * 255.0f;
					picker_data->bufferC1.SetPixel(r, b, RGB(rf, green, bf));
				}
			}
			picker_data->bufferC1.Display(hcomp);
			DrawCircle(hcomp, red, green, blue, (red / 255.0f) * widthC1, (blue / 255.0f) * heightC1);
		}

		// BLUE
		else if (IsDlgButtonChecked(hDlg, IDC_B) == BST_CHECKED)
		{
			for (int g = 0; g < widthC1; g++)
			{
				gf = (g / widthC1) * 255.0f;
				for (int r = 0; r < heightC1; r++)
				{
					rf = (r / heightC1) * 255.0f;
					picker_data->bufferC1.SetPixel(g, r, RGB(rf, gf, blue));
				}
			}
			picker_data->bufferC1.Display(hcomp);
			DrawCircle(hcomp, red, green, blue, (green / 255.0f) * widthC1, (red / 255.0f) * heightC1);
		}

		// HUE
		else if (IsDlgButtonChecked(hDlg, IDC_H) == BST_CHECKED)
		{
			double sat, val, stepsat, stepval;
			SColour tempcol;

			sat = val = 0.0;
			stepsat = 100.0 / widthC1;
			stepval = 100.0 / heightC1;

			tempcol.h = hue;
			tempcol.s = sat;
			tempcol.v = val;

			for (int y = heightC1 - 1; y > -1; y--)
			{
				for (int x = 0; x < widthC1; x++)
				{
					sat += stepsat;
					tempcol.s = sat;
					tempcol.UpdateRGB();
					picker_data->bufferC1.SetPixel(x, y, RGB(tempcol.r, tempcol.g, tempcol.b));
				}

				val += stepval;
				sat = 0.0;
				tempcol.v = val;
			}
			picker_data->bufferC1.Display(hcomp);

			// Draws circle
			tempcol.s = saturation;
			tempcol.v = value;
			tempcol.UpdateRGB();

			DrawCircle(hcomp, tempcol.r, tempcol.g, tempcol.b, tempcol.s / stepsat, heightC1 - (tempcol.v / stepval));
		}

		// SATURATION
		else if (IsDlgButtonChecked(hDlg, IDC_S) == BST_CHECKED)
		{
			double temphue, val, stepval, stephue;
			SColour tempcol;

			temphue = val = 0.0;
			stephue = 359.0 / widthC1;
			stepval = 100.0 / heightC1;

			tempcol.h = temphue;
			tempcol.s = saturation;
			tempcol.v = val;

			for (int y = heightC1 - 1; y > -1; y--)
			{
				for (int x = 0; x < widthC1; x++)
				{
					temphue += stephue;
					tempcol.h = temphue;
					tempcol.UpdateRGB();
					picker_data->bufferC1.SetPixel(x, y, RGB(tempcol.r, tempcol.g, tempcol.b));
				}

				val += stepval;
				temphue = 0.0;
				tempcol.v = val;
			}
			picker_data->bufferC1.Display(hcomp);

			// Draws circle
			tempcol.h = hue;
			tempcol.v = value;
			tempcol.UpdateRGB();

			DrawCircle(hcomp, tempcol.r, tempcol.g, tempcol.b, tempcol.h / stephue, heightC1 - (tempcol.v / stepval));
		}

		// VALUE
		else if (IsDlgButtonChecked(hDlg, IDC_V) == BST_CHECKED)
		{
			double temphue, sat, stepsat, stephue;
			SColour tempcol;

			temphue = sat = 0.0;
			stephue = 359.0 / widthC1;
			stepsat = 100.0 / heightC1;

			tempcol.h = temphue;
			tempcol.s = sat;
			tempcol.v = value;

			for (int y = heightC1 - 1; y > -1; y--)
			{
				for (int x = 0; x < widthC1; x++)
				{
					temphue += stephue;
					tempcol.h = temphue;
					tempcol.UpdateRGB();
					picker_data->bufferC1.SetPixel(x, y, RGB(tempcol.r, tempcol.g, tempcol.b));
				}

				sat += stepsat;
				temphue = 0.0;
				tempcol.s = sat;
			}
			picker_data->bufferC1.Display(hcomp);

			// Draws circle
			tempcol.h = hue;
			tempcol.s = saturation;
			tempcol.UpdateRGB();

			DrawCircle(hcomp, tempcol.r, tempcol.g, tempcol.b, tempcol.h / stephue, heightC1 - (tempcol.s / stepsat));
		}

		BitBlt(hdc, 0, 0, widthC1, heightC1, hcomp, 0, 0, SRCCOPY);

		DeleteObject(hbmp);
		DeleteDC(hcomp);
		ReleaseDC(Color1, hdc);

		// Small color selector (displays selected feature)
		hdc = GetDC(Color2);
		hcomp = CreateCompatibleDC(hdc);

		hbmp = CreateCompatibleBitmap(hdc, widthC2, heightC2);
		SelectObject(hcomp, hbmp);

		for (int y = heightC2 - 1; y > -1; y--)
		{
			for (int x = 0; x < 6; x++)
			{
				picker_data->bufferC2.SetPixel(x, y, backgroundColor.lbColor);
			}
			for (int x = widthC2 - 6; x < widthC2; x++)
			{
				picker_data->bufferC2.SetPixel(x, y, backgroundColor.lbColor);
			}
		}

		// Check who is selected.
		// RED
		if (IsDlgButtonChecked(hDlg, IDC_R) == BST_CHECKED)
		{
			for (int r = heightC2 - 1; r > -1; r--)
			{
				rf = ((r / heightC2) * 255.0f);
				for (int x = 6; x < widthC2 - 6; x++)
				{
					picker_data->bufferC2.SetPixel(x, r, RGB(255 - rf, green, blue));
				}
			}
			picker_data->bufferC2.Display(hcomp);
			DrawArrows(hcomp, widthC2, heightC2, (red / 255.0f) * heightC2);
		}
		// GREEN
		else if (IsDlgButtonChecked(hDlg, IDC_G) == BST_CHECKED)
		{
			for (int g = heightC2 - 1; g > -1; g--)
			{
				gf = ((g / heightC2) * 255.0f);
				for (int x = 6; x < widthC2 - 6; x++)
				{
					picker_data->bufferC2.SetPixel(x, g, RGB(red, 255 - gf, blue));
				}
			}
			picker_data->bufferC2.Display(hcomp);
			DrawArrows(hcomp, widthC2, heightC2, (green / 255.0f) * heightC2);
		}
		// BLUE
		else if (IsDlgButtonChecked(hDlg, IDC_B) == BST_CHECKED)
		{
			for (int b = heightC2 - 1; b > -1; b--)
			{
				bf = ((b / heightC2) * 255.0f);
				for (int x = 6; x < widthC2 - 6; x++)
				{
					picker_data->bufferC2.SetPixel(x, b, RGB(red, green, 255 - bf));
				}
			}
			picker_data->bufferC2.Display(hcomp);
			DrawArrows(hcomp, widthC2, heightC2, (blue / 255.0f) * heightC2);
		}
		// HUE
		else if (IsDlgButtonChecked(hDlg, IDC_H) == BST_CHECKED)
		{
			double temphue, step;
			SColour tempcol;

			temphue = 0.0;

			tempcol.h = 0;
			tempcol.s = 100;
			tempcol.v = 100;

			step = 359.0 / heightC2;

			for (int y = heightC2 - 1; y > -1; y--)
			{
				tempcol.UpdateRGB();

				for (int x = 6; x < widthC2 - 6; x++)
				{
					picker_data->bufferC2.SetPixel(x, y, RGB(tempcol.r, tempcol.g, tempcol.b));
				}

				temphue += step;
				tempcol.h = temphue;
			}
			picker_data->bufferC2.Display(hcomp);

			temphue = hue;
			temphue = temphue / step;

			DrawArrows(hcomp, widthC2, heightC2, temphue);
		}
		// SATURATION
		else if (IsDlgButtonChecked(hDlg, IDC_S) == BST_CHECKED)
		{
			double sat, step;
			SColour tempcol;

			sat = 0.0;

			tempcol.h = hue;
			tempcol.s = 0;
			tempcol.v = value;

			step = 100.0 / heightC2;

			for (int y = heightC2 - 1; y > -1; y--)
			{
				tempcol.UpdateRGB();

				for (int x = 6; x < widthC2 - 6; x++)
				{
					picker_data->bufferC2.SetPixel(x, y, RGB(tempcol.r, tempcol.g, tempcol.b));
				}

				sat += step;
				tempcol.s = sat;
			}
			picker_data->bufferC2.Display(hcomp);

			sat = saturation;
			sat = sat / step;

			DrawArrows(hcomp, widthC2, heightC2, sat);
		}
		// VALUE
		else if (IsDlgButtonChecked(hDlg, IDC_V) == BST_CHECKED)
		{
			double val, step;
			SColour tempcol;

			val = 0.0;

			tempcol.h = hue;
			tempcol.s = saturation;
			tempcol.v = 0;

			step = 100.0 / heightC2;

			for (int y = heightC2 - 1; y > -1; y--)
			{
				tempcol.UpdateRGB();

				for (int x = 6; x < widthC2 - 6; x++)
				{
					picker_data->bufferC2.SetPixel(x, y, RGB(tempcol.r, tempcol.g, tempcol.b));
				}

				val += step;
				tempcol.v = val;
			}
			picker_data->bufferC2.Display(hcomp);

			val = value;
			val = val / step;

			DrawArrows(hcomp, widthC2, heightC2, val);
		}

		BitBlt(hdc, 0, 0, widthC2, heightC2, hcomp, 0, 0, SRCCOPY);

		DeleteObject(hbmp);
		DeleteDC(hcomp);
		ReleaseDC(Color2, hdc);

		// Alpha slider
		hdc = GetDC(Alpha);
		hcomp = CreateCompatibleDC(hdc);

		hbmp = CreateCompatibleBitmap(hdc, widthA, heightA);
		SelectObject(hcomp, hbmp);

		for (int y = heightA - 1; y > -1; y--)
		{
			for (int x = 0; x < 6; x++)
			{
				picker_data->bufferA.SetPixel(x, y, backgroundColor.lbColor);
			}
			for (int x = widthA - 6; x < widthA; x++)
			{
				picker_data->bufferA.SetPixel(x, y, backgroundColor.lbColor);
			}
		}

		rf = red / 255.0f;
		gf = green / 255.0f;
		bf = blue / 255.0f;
		bool flag = false;

		// https://stackoverflow.com/questions/12228548/finding-equivalent-color-with-opacity#12228643
		for (int y = heightA - 1; y > -1; y--)
		{
			COLORREF cb, cw;

			if (!(y % (int)(widthA / 2 - 6)))
			{
				flag = !flag;
			}

			float af = 1.0f - (y / heightA);

			uint8_t rs = backgroundColor.lbColor & 0xFF;
			uint8_t gs = (backgroundColor.lbColor & 0xFF00) >> 8;
			uint8_t bs = (backgroundColor.lbColor & 0xFF0000) >> 16;

			const uint8_t rw = rs + ((rf * 255) - rs)*af;
			const uint8_t gw = gs + ((gf * 255) - gs)*af;
			const uint8_t bw = bs + ((bf * 255) - bs)*af;

			// Make the second color a bit darker
			rs -= 50;
			gs -= 50;
			bs -= 50;

			const uint8_t rb = rs + ((rf * 255) - rs)*af;
			const uint8_t gb = gs + ((gf * 255) - gs)*af;
			const uint8_t bb = bs + ((bf * 255) - bs)*af;

			cb = RGB(rb, gb, bb);
			cw = RGB(rw, gw, bw);

			for (int x = 6; x < (widthA / 2); x++)
			{
				picker_data->bufferA.SetPixel(x, y, flag ? cw : cb);
			}
			for (int x = (widthA / 2); x < widthA - 6; x++)
			{
				picker_data->bufferA.SetPixel(x, y, flag ? cb : cw);
			}
		}
		picker_data->bufferA.Display(hcomp);

		DrawArrows(hcomp, widthA, heightA, (SendDlgItemMessage(hDlg, IDC_ASLIDER, UDM_GETPOS, 0, (LPARAM)&result) / 255.0f) * heightA);

		BitBlt(hdc, 0, 0, widthA, heightA, hcomp, 0, 0, SRCCOPY);

		DeleteObject(hbmp);
		DeleteDC(hcomp);
		ReleaseDC(Alpha, hdc);

		DeleteObject(backgroundColorBrush);


		DrawCheckedRect(CurrentColor, picker_data->picker->GetCurrentColour().r, picker_data->picker->GetCurrentColour().g,
			picker_data->picker->GetCurrentColour().b, picker_data->picker->GetCurrentColour().a, 10, 10);

		DrawCheckedRect(OldColor, picker_data->picker->GetOldColour().r, picker_data->picker->GetOldColour().g, picker_data->picker->GetOldColour().b,
			picker_data->picker->GetOldColour().a, 10, 10);

		break;
	}

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	{
		if (uMsg == WM_MOUSEMOVE && wParam != MK_LBUTTON)
		{
			break;
		}

		POINT p;
		GetCursorPos(&p);

		// IDC_COLOR1 picked
		if (_IS_IN(rectC1.left, rectC1.right, p.x) && _IS_IN(rectC1.top, rectC1.bottom, p.y))
		{
			const float fx = ((p.x - rectC1.left) / widthC1) * 255.0f;
			const float fy = ((p.y - rectC1.top) / heightC1) * 255.0f;

			if (IsDlgButtonChecked(hDlg, IDC_R) == BST_CHECKED)
			{
				picker_data->picker->SetRGB(red, fx, fy);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_G) == BST_CHECKED)
			{
				picker_data->picker->SetRGB(fx, green, fy);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_B) == BST_CHECKED)
			{
				picker_data->picker->SetRGB(fy, fx, blue);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_H) == BST_CHECKED)
			{
				picker_data->picker->SetHSV(hue, fx / 255.0 * 100.0, (255 - fy) / 255.0 * 100.0);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_S) == BST_CHECKED)
			{
				picker_data->picker->SetHSV(fx / 255.0 * 359.0, saturation, (255 - fy) / 255.0 * 100.0);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_V) == BST_CHECKED)
			{
				picker_data->picker->SetHSV(fx / 255.0 * 359.0, (255 - fy) / 255.0 * 100.0, value);
			}
		}
		// IDC_COLOR2 picked
		else if (_IS_IN(rectC2.left, rectC2.right, p.x) && _IS_IN(rectC2.top, rectC2.bottom, p.y))
		{
			const float fy = ((p.y - rectC2.top) / heightC2) * 255.0f;

			if (IsDlgButtonChecked(hDlg, IDC_R) == BST_CHECKED)
			{
				picker_data->picker->SetRGB(255 - fy, green, blue);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_G) == BST_CHECKED)
			{
				picker_data->picker->SetRGB(red, 255 - fy, blue);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_B) == BST_CHECKED)
			{
				picker_data->picker->SetRGB(red, green, 255 - fy);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_H) == BST_CHECKED)
			{
				picker_data->picker->SetHSV((255 - fy) / 255.0 * 359.0, saturation, value);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_S) == BST_CHECKED)
			{
				picker_data->picker->SetHSV(hue, (255 - fy) / 255.0 * 100.0, value);
			}

			else if (IsDlgButtonChecked(hDlg, IDC_V) == BST_CHECKED)
			{
				picker_data->picker->SetHSV(hue, saturation, (255 - fy) / 255.0 * 100.0);
			}
		}
		// IDC_ALPHASLIDE picked
		else if (_IS_IN(rectA.left, rectA.right, p.x) && _IS_IN(rectA.top, rectA.bottom, p.y))
		{
			const float fy = ((p.y - rectA.top) / heightA) * 255.0f;

			picker_data->picker->SetAlpha(255 - fy);
		}

		UpdateValues(hDlg, picker_data->picker->GetCurrentColour(), picker_data->changing_text);
		RedrawWindow(hDlg, NULL, NULL, RDW_UPDATENOW | RDW_INTERNALPAINT);
		break;
	}

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case EN_SETFOCUS:
		{
			if (LOWORD(wParam) != IDC_HEXCOL)
			{
				if (GetDlgItemInt(hDlg, LOWORD(wParam), NULL, FALSE) == 0)
				{
					SetDlgItemText(hDlg, LOWORD(wParam), L"");
				}
			}
			SendDlgItemMessage(hDlg, LOWORD(wParam), EM_SETSEL, 0, -1);
			break;
		}

		case EN_KILLFOCUS:
		{
			if (LOWORD(wParam) == IDC_HEXCOL)
			{
				wchar_t rawText[11];
				GetDlgItemText(hDlg, IDC_HEXCOL, rawText, 11);

				std::wstring text(rawText);
				if (text.find_first_of('#') == 0)
				{
					text = text.substr(1, text.length() - 1);
				}
				else if (text.find(L"0x") == 0)
				{
					text = text.substr(2, text.length() - 2);
				}

				try
				{
					const unsigned int tempcolor = std::stoll(text, nullptr, 16) & 0xFFFFFFFF;

					if (text.length() == 8)
					{
						picker_data->picker->SetRGB((tempcolor & 0xFF000000) >> 24, (tempcolor & 0x00FF0000) >> 16, (tempcolor & 0x0000FF00) >> 8);
						picker_data->picker->SetAlpha(tempcolor & 0x000000FF);
					}
					else if (text.length() == 4)
					{
						picker_data->picker->SetRGB((tempcolor & 0xF000) >> 12, (tempcolor & 0x0F00) >> 8, (tempcolor & 0x00F0) >> 4);
						picker_data->picker->SetAlpha(tempcolor & 0x000F);
					}
					else if (text.length() == 6)
					{
						picker_data->picker->SetRGB((tempcolor & 0xFF0000) >> 16, (tempcolor & 0x00FF00) >> 8, tempcolor & 0x0000FF);
					}
					else if (text.length() == 3)
					{
						picker_data->picker->SetRGB((tempcolor & 0xF00) >> 8, (tempcolor & 0x0F0) >> 4, tempcolor & 0x00F);
					}

					UpdateValues(hDlg, picker_data->picker->GetCurrentColour(), picker_data->changing_text);
					RedrawWindow(hDlg, NULL, NULL, RDW_UPDATENOW | RDW_INTERNALPAINT);
				}
				catch (std::invalid_argument) { }
			}

			picker_data->changing_text = true;
			for (const std::pair<const unsigned int, std::pair<unsigned int, unsigned int>> &slider_combo : SLIDER_MAP)
			{
				if (slider_combo.first == IDC_HEXCOL)
				{
					wchar_t buffer[11];
					SColour col = picker_data->picker->GetCurrentColour();

					swprintf_s(buffer, L"0x%02X%02X%02X%02X", col.r, col.g, col.b, col.a);
					SetDlgItemText(hDlg, slider_combo.first, buffer);
				}
				else
				{
					SetDlgItemInt(hDlg, slider_combo.first, SendDlgItemMessage(hDlg, slider_combo.second.first, UDM_GETPOS, 0, (LPARAM)&result), false);
				}
			}
			picker_data->changing_text = false;
			break;
		}

		case EN_CHANGE:
		{
			if (picker_data->changing_text)
			{
				break;
			}

			switch (LOWORD(wParam))
			{
			case IDC_RED:
			case IDC_GREEN:
			case IDC_BLUE:
			{
				picker_data->picker->SetRGB(SendDlgItemMessage(hDlg, IDC_RSLIDER, UDM_GETPOS, 0, (LPARAM)&result), SendDlgItemMessage(hDlg, IDC_GSLIDER, UDM_GETPOS, 0, (LPARAM)&result), SendDlgItemMessage(hDlg, IDC_BSLIDER, UDM_GETPOS, 0, (LPARAM)&result));
				break;
			}

			case IDC_HUE:
			case IDC_SATURATION:
			case IDC_VALUE:
			{
				picker_data->picker->SetHSV(SendDlgItemMessage(hDlg, IDC_HSLIDER, UDM_GETPOS, 0, (LPARAM)&result), SendDlgItemMessage(hDlg, IDC_SSLIDER, UDM_GETPOS, 0, (LPARAM)&result), SendDlgItemMessage(hDlg, IDC_VSLIDER, UDM_GETPOS, 0, (LPARAM)&result));
				break;
			}

			case IDC_ALPHA:
			{
				picker_data->picker->SetAlpha(SendDlgItemMessage(hDlg, IDC_ASLIDER, UDM_GETPOS, 0, (LPARAM)&result));
				break;
			}

			case IDC_HEXCOL:
			{
				return 0;
			}
			}

			// Update color
			UpdateValues(hDlg, picker_data->picker->GetCurrentColour(), picker_data->changing_text);
			RedrawWindow(hDlg, NULL, NULL, RDW_UPDATENOW | RDW_INTERNALPAINT);

			break;
		}

		case BN_CLICKED: // Equivalent to STN_CLICKED
		{
			switch (LOWORD(wParam))
			{
			case IDC_R:
			case IDC_B:
			case IDC_G:
			case IDC_H:
			case IDC_S:
			case IDC_V:
			{
				RedrawWindow(hDlg, NULL, NULL, RDW_UPDATENOW | RDW_INTERNALPAINT);
				break;
			}

			case IDC_OLDCOLOR:
			{
				SColour old = picker_data->picker->GetOldColour();

				picker_data->picker->SetRGB(old.r, old.g, old.b);
				picker_data->picker->SetAlpha(old.a);
				UpdateValues(hDlg, picker_data->picker->GetCurrentColour(), picker_data->changing_text);
				RedrawWindow(hDlg, NULL, NULL, RDW_UPDATENOW | RDW_INTERNALPAINT);
				break;
			}

			case IDB_OK:
			{
				picker_data->picker->UpdateOldColour();
				EndDialog(hDlg, IDB_OK);
				break;
			}

			case IDB_CANCEL:
			{
				SColour old = picker_data->picker->GetOldColour();

				picker_data->picker->SetRGB(old.r, old.g, old.b);
				picker_data->picker->SetAlpha(old.a);
				EndDialog(hDlg, IDB_CANCEL);
				break;
			}
			}

			break;
		}
		}
		break;
	}
	return 0;
}

// Draw a b/w checked rectangle, "covered" with the rgba color provided.
// cx and cy are the size of the checks
void DrawCheckedRect(HWND hWnd, int r, int g, int b, int a, int cx, int cy)
{
	float rf = (float)r / 255.0f,
		gf = (float)g / 255.0f,
		bf = (float)b / 255.0f,
		af = (float)a / 255.0f;
	HDC hdc = GetDC(hWnd);
	HBRUSH brush, brush2;
	RECT rect, r2;
	bool flag;

	brush = CreateSolidBrush(RGB((rf*af) * 255, (gf*af) * 255, (bf*af) * 255));
	brush2 = CreateSolidBrush(RGB((rf*af + 1 - af) * 255, (gf*af + 1 - af) * 255, (bf*af + 1 - af) * 255));

	GetWindowRect(hWnd, &rect);

	for (int x = 0; (x*cx) < WIDTH(rect); x++)
	{
		if (x % 2)
			flag = false;
		else
			flag = true;
		for (int y = 0; (y*cy) < HEIGHT(rect); y++)
		{
			r2.left = x * cx;
			r2.right = (std::min<long>)((x + 1)*cx, WIDTH(rect) - 2);
			r2.top = y * cy;
			r2.bottom = (std::min<long>)((y + 1)*cy, HEIGHT(rect) - 2);

			FillRect(hdc, &r2, flag ? brush : brush2);

			flag = !flag;
		}
	}

	DeleteObject(brush);
	DeleteObject(brush2);
	ReleaseDC(hWnd, hdc);
}

void DrawCircle(HDC hcomp, int red, int green, int blue, float x, float y)
{
	HPEN pen = CreatePen(PS_SOLID, 1, RGB(255 - red, 255 - green, 255 - blue));
	HGDIOBJ prev = SelectObject(hcomp, pen);
	Arc(hcomp, x - 5, y - 5, x + 5, y + 5, 0, 0, 0, 0);
	SelectObject(hcomp, prev);
	DeleteObject(pen);
}

void DrawArrows(HDC hcomp, int width, int height, float y)
{
	HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HGDIOBJ prev = SelectObject(hcomp, pen);
	MoveToEx(hcomp, 0, height - (y - 5), NULL);
	LineTo(hcomp, 0, height - (y + 5));
	LineTo(hcomp, 5, height - (y));
	LineTo(hcomp, 0, height - (y - 5));
	MoveToEx(hcomp, width - 1, height - (y - 5), NULL);
	LineTo(hcomp, width - 1, height - (y + 5));
	LineTo(hcomp, width - 6, height - (y));
	LineTo(hcomp, width - 1, height - (y - 5));
	SelectObject(hcomp, prev);
	DeleteObject(pen);
}

void UpdateValues(HWND hDlg, SColour col, bool &changing_text)
{
	changing_text = true;

	SendDlgItemMessage(hDlg, IDC_RSLIDER, UDM_SETPOS, 0, col.r);
	SendDlgItemMessage(hDlg, IDC_GSLIDER, UDM_SETPOS, 0, col.g);
	SendDlgItemMessage(hDlg, IDC_BSLIDER, UDM_SETPOS, 0, col.b);
	SendDlgItemMessage(hDlg, IDC_ASLIDER, UDM_SETPOS, 0, col.a);
	SendDlgItemMessage(hDlg, IDC_HSLIDER, UDM_SETPOS, 0, col.h);
	SendDlgItemMessage(hDlg, IDC_SSLIDER, UDM_SETPOS, 0, col.s);
	SendDlgItemMessage(hDlg, IDC_VSLIDER, UDM_SETPOS, 0, col.v);
	SendDlgItemMessage(hDlg, IDC_HEXSLIDER, UDM_SETPOS32, 0, (col.r << 24) + (col.g << 16) + (col.b << 8) + col.a);

	changing_text = false;
}