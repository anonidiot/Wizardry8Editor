/*
 * Copyright (C) 2022 Anonymous Idiot
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Wizardry8Style.h"
#include "Wizardry8Scalable.h"
#include <qdrawutil.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QIcon>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTransform>
#include "WListWidget.h"
#include "WSpinBox.h"
#include "WindowItemsList.h"

#include "SLFFile.h"
#include "STItoQImage.h"

#include "qdebug.h"

#define CB_OFFSET         5
#define SPIN_SUB_OFFSET  25
#define SPIN_ADD_OFFSET  30

#define PM_SpinnerHeight       (QStyle::PixelMetric)((int)PM_CustomBase + 1) // Height of the +/- pixmap on a spinner
#define PM_SpinnerWidth        (QStyle::PixelMetric)((int)PM_CustomBase + 2) // Width of the +/- pixmap on a spinner
#define PM_SpinnerDeltaWidth   (QStyle::PixelMetric)((int)PM_CustomBase + 3) // Width of the secondary number field showing +/- adjustment
#define PM_SpinnerTextSpacer   (QStyle::PixelMetric)((int)PM_CustomBase + 4) // horizontal space between number fields and the +/- pixmap

#define SC_SpinBoxDelta        (QStyle::SubControl)((int)QStyle::SubControl::SC_CustomBase + 1) // Secondary number field to show the +/- total adjustment

#define CE_Slider              (QStyle::ControlElement)((int)CE_CustomBase + 1) // drawing routine for handle on QSlider

#define SC_DELTA_OBJECT_NAME   "qt_spinbox_delta"

Wizardry8Style::Wizardry8Style() :
    QProxyStyle(QStyleFactory::create("windows")),
    m_up_arrow(NULL),
    m_down_arrow(NULL),
    m_sbslider(NULL),
    m_slider(NULL),
    m_cb(NULL),
    m_spinner(NULL)
{
    setObjectName("Wizardry8");

    // For scrollbars
    SLFFile up_arrow( "DIALOGS/DIALOGUPARROW.STI" );
    if (up_arrow.open(QFile::ReadOnly))
    {
        QByteArray array = up_arrow.readAll();
        m_up_arrow = new STItoQImage( array );
        up_arrow.close();
    }
    SLFFile down_arrow( "DIALOGS/DIALOGDOWNARROW.STI" );
    if (down_arrow.open(QFile::ReadOnly))
    {
        QByteArray array = down_arrow.readAll();
        m_down_arrow = new STItoQImage( array );
        down_arrow.close();
    }
    SLFFile sbslider( "DIALOGS/DIALOGSLIDEBAR.STI" );
    if (sbslider.open(QFile::ReadOnly))
    {
        QByteArray array = sbslider.readAll();
        m_sbslider = new STItoQImage( array );
        sbslider.close();
    }
    // For QSliders
    SLFFile slider( "OPTIONS/OPTIONS_SLIDER.STI" );
    if (slider.open(QFile::ReadOnly))
    {
        QByteArray array = slider.readAll();
        m_slider = new STItoQImage( array );
        slider.close();
    }

    // For checkboxes
    SLFFile cb( "CHAR GENERATION/CG_PERSONALITY.STI" );
    if (cb.open(QFile::ReadOnly))
    {
        QByteArray array = cb.readAll();
        m_cb = new STItoQImage( array );
        cb.close();
    }

    // For Spinboxes
    SLFFile spinner( "CHAR GENERATION/CG_BUTTONS.STI" );
    if (spinner.open(QFile::ReadOnly))
    {
        QByteArray array = spinner.readAll();
        m_spinner = new STItoQImage( array );
        spinner.close();
    }
}

Wizardry8Style::~Wizardry8Style()
{
    if (m_up_arrow)   delete m_up_arrow;
    if (m_down_arrow) delete m_down_arrow;
    if (m_sbslider)   delete m_sbslider;
    if (m_slider)     delete m_slider;
    if (m_cb)         delete m_cb;
    if (m_spinner)    delete m_spinner;
}

QPalette Wizardry8Style::standardPalette() const
{
    if (!m_standardPalette.isBrushSet(QPalette::Disabled, QPalette::Mid))
    {
        QColor navy        (0x18, 0x28, 0x63); // background of text boxes
        QColor lt_yellow   (0xe0, 0xe0, 0xc3); // main text colour
        QColor gr_yellow   (0x70, 0x70, 0x43); // disabled text colour
        QColor orange_brn  (0xef, 0x9c, 0x52); // frame of tooltips

        QPalette palette;

        // This is a very icon heavy style - it doesn't use a whole lot
        // of text, and never on buttons, which are always icons.
        palette.setBrush(QPalette::Base,            Qt::black);
        palette.setBrush(QPalette::Button,          Qt::black);
        palette.setBrush(QPalette::ButtonText,      lt_yellow);
        palette.setBrush(QPalette::Window,          Qt::black);
        palette.setBrush(QPalette::WindowText,      lt_yellow);
        palette.setBrush(QPalette::Highlight,       Qt::blue);
        palette.setBrush(QPalette::HighlightedText, lt_yellow);
        palette.setBrush(QPalette::Text,            lt_yellow);
        palette.setBrush(QPalette::Light,           gr_yellow);

        // Using AlternateBase instead of Base to minimise
        // the number of widgets that will try to use it.
        palette.setBrush(QPalette::AlternateBase,   navy);

        QBrush brush = palette.window();
        brush.setColor(brush.color().darker());

        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Text,       gr_yellow);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base,       brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button,     brush);
        palette.setBrush(QPalette::Disabled, QPalette::Mid,        brush);

        palette.setBrush(QPalette::Midlight,    orange_brn);
        palette.setBrush(QPalette::ToolTipBase, Qt::black);
        palette.setBrush(QPalette::ToolTipText, Qt::white);

        m_standardPalette = palette;
    }

    return m_standardPalette;
}

SpinBoxExtra::SpinBoxExtra(QSpinBox *superControl) :
   m_super(superControl),
   m_inited(false)
{
    m_delta = new QLineEdit(m_super);
    m_delta->setObjectName(SC_DELTA_OBJECT_NAME);

    m_delta->setReadOnly(true);
    m_delta->setAlignment( (m_super->alignment() & Qt::AlignVertical_Mask) | Qt::AlignRight );

    QStyleOptionSpinBox opt;
    opt.initFrom(m_super);
    opt.subControls = SC_SpinBoxDelta;

    m_delta->setGeometry(m_super->style()->subControlRect(QStyle::CC_SpinBox, &opt, SC_SpinBoxDelta, m_super));
    valueChanged( m_super->value() );
}

SpinBoxExtra::~SpinBoxExtra()
{
    // m_delta will be cleaned up by the normal widget cleanup that occurs when QSpinBox is deleted
    // segfaults will occur if we try to delete or deleteLater it in here.
}

void SpinBoxExtra::valueChanged(int value)
{
    if (WSpinBox *w = qobject_cast<WSpinBox *>(m_super))
    {
        m_inited        = true;
        m_initialValue  = w->m_initialValue;
    }
    else if (m_inited == false)
    {
        // If not a custom SpinBox with methods for setting the delta,
        // then default to the first value set
        m_inited       = true;
        m_initialValue = value;
    }

    m_delta->setText( QString::number( value - m_initialValue ) );
    m_delta->show();
}

void Wizardry8Style::polish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget) ||
        qobject_cast<QScrollBar *> (widget) ||
        qobject_cast<QCheckBox *>  (widget) ||
        qobject_cast<QSpinBox *>   (widget) ||
        qobject_cast<QSlider *>   (widget))
    {
        widget->setAttribute(Qt::WA_Hover, true);
    }

    // This is a hack - a completely inappropriate way to modify the behaviour of
    // the SpinBox widget without subclassing it, but that's the intent here - to
    // keep all the changes confined solely to the style so that normal SpinBoxes
    // will work in a Wizardry 8 fashion without any modifications at all, and
    // conversely this style can be swapped out without having customised widgets
    // that won't work with other styles.
    // What we are doing is creating our own private class with an autodestructor
    // that should cause it to be cleaned up when the SpinBox it is tied to is.
    // Internally it monitors the value changes of the spinner and shows a text
    // field to indicate the delta from the original value. A limitation of this
    // technique is that there isn't a way to reset the delta to 0 if a new value
    // is explicitly set by the application - it only gets the first one applied.
    // (I actually did end up subclassing after all in order to get around this
    //  and a separate scaling problem, but unmodified QSpinners should still
    //  otherwise still work semi-correctly with this style.)
    if (QSpinBox *q = qobject_cast<QSpinBox *>   (widget))
    {
        SpinBoxExtra *delta = new SpinBoxExtra( q );

        QObject::connect( q, SIGNAL(destroyed()), delta, SLOT(parentDestroyed()));
        QObject::connect( q, SIGNAL(valueChanged(int)), delta, SLOT(valueChanged(int)));
    }

    if (qobject_cast<WindowItemsList *>(widget))
    {
        // This widget wants to setup a tiled background; it can't if the
        // palette gets reset here.
    }
    else
    {
        // irrespective of widget FORCE the bloody thing to use our palette
        // because they're all ignoring it and using their own even when the
        // app is set to use the style palette!
        // Shouldn't have to do this!!!!
        widget->setPalette( m_standardPalette );
    }
}

void Wizardry8Style::unpolish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget) ||
        qobject_cast<QScrollBar *> (widget) ||
        qobject_cast<QCheckBox *>  (widget) ||
        qobject_cast<QSpinBox *>   (widget))
    {
        widget->setAttribute(Qt::WA_Hover, false);
    }
}

double Wizardry8Style::getParentScale(const QWidget *widget) const
{
    if (widget != NULL)
    {
        // Different windows could end up having different scales, so use
        // a Wizardry8Scalable interface to hold the scale used by each one
        for (QObject *p = (QObject *)widget; p; p = p->parent())
        {
            // We don't use qobject_cast here because Wizardry8Scalable doesn't
            // inherit from QObject (is intended to function as an interface on
            // objects that already do)
            if (const Wizardry8Scalable *sc = dynamic_cast<const Wizardry8Scalable *>(p))
            {
                return sc->getScale();
            }
        }
    }

    return 1.0;
}

int Wizardry8Style::pixelMetric(PixelMetric metric,
                                const QStyleOption *option,
                                const QWidget *widget) const
{
    double scale = getParentScale(widget);

    switch (metric)
    {
        // For checkbox
        case PM_IndicatorHeight:
            return (int)(scale * m_cb->getHeight(CB_OFFSET));

        case PM_IndicatorWidth:
            return (int)(scale * m_cb->getWidth(CB_OFFSET));

        // For scrollbar
        case PM_ScrollBarExtent: // used for width of vertical scrollbars, and height of horiz ones
            return (int)(scale * m_down_arrow->getWidth(0));

        case PM_ScrollBarSliderMin:
            return (int)(scale * m_sbslider->getHeight(0));

        // QSlider
        case PM_SliderThickness:
            return (int)(scale * 12);

        case PM_SliderLength:
            return (int)(scale * m_slider->getHeight(1));

        case PM_SliderControlThickness:
            return (int)(scale * m_slider->getWidth(1));

        // For buttons
        case PM_DefaultFrameWidth:
        {
            if (widget != NULL)
            {
                if (qobject_cast<const QPushButton *>(widget))
                {
                    return 0;
                }
            }
            return QProxyStyle::pixelMetric(metric, option, widget);
        }

        case PM_ButtonShiftHorizontal:
        case PM_ButtonShiftVertical:
        case PM_ButtonDefaultIndicator:
        case PM_ButtonMargin:
            return 0;

        case PM_ButtonIconSize:
        {
            // This only lets you return a single dimension of the icon.
            // If the setIconSize() method is called on the QPushButton
            // BOTH correct dimensions can be used, though, by bypassing
            // this query - SO DO THAT!
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                if (!button->icon.isNull())
                {
                    QSize ic = button->icon.actualSize(QSize(65536, 65536));

                    return (int)ic.height(); // not multiplied by scale because might be called as part of CT_PushButton
                }
            }
            return QProxyStyle::pixelMetric(metric, option, widget);
        }

        // Custom PMs
        /////////////

// We know we're abusing the enum type here, so shutup the compiler warning about it
#pragma GCC diagnostic ignored "-Wswitch"

        // For SpinBox
        case PM_SpinnerHeight:
            return (int)(scale * m_spinner->getHeight(SPIN_ADD_OFFSET));

        case PM_SpinnerWidth:
            return (int)(scale * m_spinner->getWidth(SPIN_ADD_OFFSET));

        case PM_SpinnerDeltaWidth:
            return (int)(scale * 25); // 18 looks more correct for single +/- 9 at most deltas

        case PM_SpinnerTextSpacer:
            return (int)(scale * 4);
#pragma GCC diagnostic pop

        default:
            return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QSize Wizardry8Style::sizeFromContents(ContentsType ct,
                                       const QStyleOption *opt,
                                       const QSize &csz,
                                       const QWidget *widget) const
{
    switch (ct)
    {
        case CT_PushButton:
        {
            // Ignore everything we've been given and concentrate solely
            // on the icon
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(opt))
            {
                if (!button->icon.isNull())
                    return (button->iconSize * getParentScale(widget));
            }
            return QProxyStyle::sizeFromContents(ct, opt, csz, widget);
        }

        case CT_ItemViewItem:
        {
            if (const WListWidget *w = qobject_cast<const WListWidget *>(widget))
            {
                if (w->useTextColorInsteadOfCheckmarks())
                {
                    if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt))
                    {
                        QStyleOptionViewItem mod = *vopt;

                        // It messes up the item size calculation if this is left on
                        mod.features &= ~QStyleOptionViewItem::HasCheckIndicator;

                        return QProxyStyle::sizeFromContents(ct, &mod, csz, widget);
                    }
                }
            }
            return QProxyStyle::sizeFromContents(ct, opt, csz, widget);
        }

        default:
            return QProxyStyle::sizeFromContents(ct, opt, csz, widget);
    }
}

QRect Wizardry8Style::subControlRect(ComplexControl control,
                                     const QStyleOptionComplex *option,
                                     SubControl subControl,
                                     const QWidget *widget) const
{
    QRect rect = QCommonStyle::subControlRect(control, option, subControl, widget);

    switch (control)
    {
        case CC_ScrollBar:
        {
            const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option);

            int scrollBarExtent = pixelMetric(PM_ScrollBarExtent, scrollBar, widget);
            // Constrain the slider to be a fixed size
            int sliderLength    = pixelMetric(PM_ScrollBarSliderMin, scrollBar, widget);
            int grooveLength    =  ((scrollBar->orientation == Qt::Horizontal)
                                    ? (scrollBar->rect.width() - scrollBarExtent*2)
                                    : (scrollBar->rect.height() - scrollBarExtent*2));

            int sliderStart = scrollBarExtent +
                              sliderPositionFromValue(scrollBar->minimum,
                                                      scrollBar->maximum,
                                                      scrollBar->sliderPosition,
                                                      grooveLength - sliderLength,
                                                      scrollBar->upsideDown);

            QRect scrollBarRect = scrollBar->rect;

            switch (subControl)
            {
                case SC_ScrollBarSubLine:
                    rect.setRect(scrollBarRect.left(),
                                 scrollBarRect.top(),
                                 scrollBarExtent,
                                 scrollBarExtent);
                    break;

                case SC_ScrollBarAddLine:
                    if (scrollBar->orientation == Qt::Horizontal)
                    {
                        rect.setRect(scrollBarRect.right() - scrollBarExtent,
                                     scrollBarRect.top(),
                                     scrollBarExtent,
                                     scrollBarExtent);
                    }
                    else
                    {
                        rect.setRect(scrollBarRect.left(),
                                     scrollBarRect.bottom() - scrollBarExtent,
                                     scrollBarExtent,
                                     scrollBarExtent);
                    }
                    break;

                case SC_ScrollBarSubPage:
                    if (scrollBar->orientation == Qt::Horizontal)
                    {
                        rect.setRect(scrollBarRect.left() + scrollBarExtent,
                                     scrollBarRect.top(),
                                     sliderStart - (scrollBarRect.left() + scrollBarExtent),
                                     scrollBarExtent);
                    }
                    else
                    {
                        rect.setRect(scrollBarRect.left(),
                                     scrollBarRect.top() + scrollBarExtent,
                                     scrollBarExtent,
                                     sliderStart - (scrollBarRect.top() + scrollBarExtent));
                    }
                    break;

                case SC_ScrollBarAddPage:
                    if (scrollBar->orientation == Qt::Horizontal)
                    {
                        rect.setRect(sliderStart + sliderLength,
                                     scrollBarRect.top(),
                                     scrollBarRect.right() - scrollBarExtent - (sliderStart + sliderLength),
                                     scrollBarExtent);
                    }
                    else
                    {
                        rect.setRect(scrollBarRect.left(),
                                     sliderStart + sliderLength,
                                     scrollBarExtent,
                                     scrollBarRect.bottom() - scrollBarExtent - (sliderStart + sliderLength));
                    }
                    break;

                case SC_ScrollBarGroove:
                    if (scrollBar->orientation == Qt::Horizontal)
                    {
                        rect = scrollBarRect.adjusted(scrollBarExtent, 0, -scrollBarExtent, 0);
                    }
                    else
                    {
                        rect = scrollBarRect.adjusted(0, scrollBarExtent, 0, -scrollBarExtent);
                    }
                    break;

                case SC_ScrollBarSlider:
                    if (scrollBar->orientation == Qt::Horizontal)
                    {
                        rect.setRect(sliderStart, scrollBarRect.top(), sliderLength, scrollBarExtent);
                    }
                    else
                    {
                        rect.setRect(scrollBarRect.left(), sliderStart, scrollBarExtent, sliderLength);
                    }
                    break;

                default:
                    return QProxyStyle::subControlRect(control, option, subControl, widget);
            }
            break;
        }

        case CC_Slider:
        {
            const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option);
            int thickness = proxy()->pixelMetric(PM_SliderThickness, slider, widget);

            switch (subControl)
            {
                case SC_SliderGroove:
                    if (slider->orientation == Qt::Horizontal)
                        rect.setRect( slider->rect.x(),
                                      slider->rect.y() + (slider->rect.height() - thickness) / 2,
                                      slider->rect.width() - 1,
                                      thickness );
                    else
                        rect.setRect( slider->rect.x() + (slider->rect.width() - thickness) / 2,
                                      slider->rect.y(),
                                      thickness,
                                      slider->rect.height() - 1);
                    break;

                default:
                    return QProxyStyle::subControlRect(control, option, subControl, widget);
            }
            break;
        }

        case CC_SpinBox:
        {
            const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(option);
            int spinnerButtonWidth   = pixelMetric(PM_SpinnerWidth, spinbox, widget);
            int spinnerButtonHeight  = pixelMetric(PM_SpinnerHeight, spinbox, widget);
            int spinnerDeltaBoxWidth = pixelMetric(PM_SpinnerDeltaWidth, spinbox, widget);
            int spinnerTextSpacer    = pixelMetric(PM_SpinnerTextSpacer, spinbox, widget);

            QRect spinboxRect = spinbox->rect;

            switch (subControl)
            {
                case SC_SpinBoxUp:
                {
                    rect.setRect(spinboxRect.right() - spinnerButtonWidth,
                                 spinboxRect.top() + (spinboxRect.height() - spinnerButtonHeight)/2,
                                 spinnerButtonWidth,
                                 spinnerButtonHeight);
                    break;
                }
                case SC_SpinBoxDown:
                {
                    rect.setRect(spinboxRect.right() - spinnerButtonWidth*2 - spinnerTextSpacer - spinnerDeltaBoxWidth,
                                 spinboxRect.top() + (spinboxRect.height() - spinnerButtonHeight)/2,
                                 spinnerButtonWidth,
                                 spinnerButtonHeight);
                    break;
                }
// We know we're abusing the enum type here, so shutup the compiler warning about it
#pragma GCC diagnostic ignored "-Wswitch"
                case SC_SpinBoxDelta:
                {
                    rect.setRect(spinboxRect.right() - spinnerButtonWidth - spinnerTextSpacer - spinnerDeltaBoxWidth,
                                 spinboxRect.top(),
                                 spinnerDeltaBoxWidth,
                                 spinboxRect.height());
                    break;
                }
#pragma GCC diagnostic pop
                case SC_SpinBoxEditField:
                {
                    rect.setRect(spinboxRect.left(),
                                 spinboxRect.top(),
                                 spinboxRect.width() - spinnerButtonWidth*2 - spinnerTextSpacer*2 - spinnerDeltaBoxWidth,
                                 spinboxRect.height());

                    // More hackery -- because the base QSpinClass doesn't know about our secondary textfield
                    // it doesn't update it when the widget resizes, so every time we update the main textfield
                    // here that it does know about, we recalculate the other one too.
                    if (widget != NULL)
                    {
                        if (const QSpinBox *spinbox = qobject_cast<const QSpinBox *> (widget))
                        {
                            QList<QObject *> kids = spinbox->children();

                            for (int k=0; k<kids.size(); k++)
                            {
                                if (QLineEdit *x = qobject_cast<QLineEdit *>(kids[k]))
                                {
                                    // turn off the ability to highlight text in all the textboxes childed to spinbox
                                    x->setReadOnly(true);
                                    if (0 == x->objectName().compare(SC_DELTA_OBJECT_NAME))
                                    {
                                        x->setGeometry(subControlRect(QStyle::CC_SpinBox, option, SC_SpinBoxDelta, widget));
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
                default:
                    break;
            }
            break;
        }

        default:
            return QProxyStyle::subControlRect(control, option, subControl, widget);
    }
    return rect;
}

void Wizardry8Style::drawPrimitive(PrimitiveElement element,
                                   const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const
{
    painter->setRenderHint( QPainter::Antialiasing,          true );
    painter->setRenderHint( QPainter::TextAntialiasing,      true );
    painter->setRenderHint( QPainter::SmoothPixmapTransform, true );

    switch (element)
    {
        // QToolTip
        case PE_PanelTipLabel:
        {
            const QBrush brush(option->palette.toolTipBase());
            qDrawPlainRect(painter, option->rect, option->palette.midlight().color(), 2, &brush);
            break;
        }

        // QLineEdit
        case PE_PanelLineEdit:
        {
            if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option))
            {
                bool fill = true;

                if (widget != NULL)
                {
                    if (const QLineEdit *le = qobject_cast<const QLineEdit *> (widget))
                    {
                        fill = (le->isReadOnly() ? false : le->hasFocus());
                    }
                }

                if (fill)
                {
                    painter->fillRect(
                        panel->rect.adjusted(panel->lineWidth, panel->lineWidth, -panel->lineWidth, -panel->lineWidth),
                        panel->palette.brush(QPalette::AlternateBase));
                }
            }
            break;
        }

        // QSpinBox
        case PE_IndicatorSpinMinus:
        case PE_IndicatorSpinPlus:
        {
            QPixmap  pix;
            int state = 0; // normal

            if      (!(option->state & State_Enabled))   state = 3;
            else if   (option->state & State_MouseOver)
            {
                if    (option->state & State_Sunken)     state = 4; // clicked and mouseover
                else                                     state = 1; // mouseover
            }
            else if   (option->state & State_Sunken)     state = 2; // clicked

            if (element == PE_IndicatorSpinMinus)
                pix = QPixmap::fromImage( m_spinner->getImage( SPIN_SUB_OFFSET + state ) );
            else
                pix = QPixmap::fromImage( m_spinner->getImage( SPIN_ADD_OFFSET + state ) );

            painter->drawPixmap(option->rect, pix);
            break;
        }

        // QCheckbox
        case PE_IndicatorCheckBox:
        {
            QPixmap  pix;
            int state = 0; // normal

            if      (!(option->state & State_Enabled))   state = 0; // don't have a disabled icon
            else if   (option->state & State_MouseOver)
            {
                if    (option->state & State_Off)        state = 1; // mouseover
                else                                     state = 3; // clicked and mouseover
            }
            else if (!(option->state & State_Off))       state = 2; // clicked

            pix = QPixmap::fromImage( m_cb->getImage( CB_OFFSET + state ) );
            painter->drawPixmap(option->rect, pix);
            break;
        }

        // QScrollBar
        case PE_IndicatorArrowUp:
        case PE_IndicatorArrowDown:
        case PE_IndicatorArrowLeft:
        case PE_IndicatorArrowRight:
        {
            QPixmap  pix;
            int state = 0; // normal

            if      (!(option->state & State_Enabled))   state = 3; // disabled
            else if   (option->state & State_MouseOver)  state = 1; // mouseover
            else if   (option->state & State_Sunken)     state = 2; // depressed

            if (element == PE_IndicatorArrowUp)
            {
                pix = QPixmap::fromImage( m_up_arrow->getImage( state ) );
            }
            else if (element == PE_IndicatorArrowDown)
            {
                pix = QPixmap::fromImage( m_down_arrow->getImage( state ) );
            }
            // Sub-optimal code: Wizardry 8 UI doesn't use horizontal
            // scrollbars, and this only done for completeness
            else if (element == PE_IndicatorArrowLeft)
            {
                QMatrix rot;
                rot.rotate(270);
                QImage left_arrow = m_up_arrow->getImage( state ).transformed( QTransform(rot) );
                pix = QPixmap::fromImage( left_arrow );
            }
            else if (element == PE_IndicatorArrowRight)
            {
                QMatrix rot;
                rot.rotate(270);
                QImage right_arrow = m_down_arrow->getImage( state ).transformed( QTransform(rot) );
                pix = QPixmap::fromImage( right_arrow );
            }

            painter->drawPixmap(option->rect, pix);
            break;
        }

        // QListView
        case PE_PanelItemViewItem:
        {
            if (const QStyleOptionViewItem *view = qstyleoption_cast<const QStyleOptionViewItem *>(option))
            {
                if (view->features & QStyleOptionViewItem::Alternate)
                {
                    // Don't render this primitive
                    break;
                }
            }
            QProxyStyle::drawPrimitive(element, option, painter, widget);
            break;
        }

        // Everything
        case PE_FrameFocusRect:
        {
            // This style is only intended for mouse use, making focus
            // irrelevant. Don't encourage keyboard.
            // draw nothing - because it looks awful
            break;
        }

        default:
            QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

void Wizardry8Style::drawControl(ControlElement element,
                                 const QStyleOption *option,
                                 QPainter *painter,
                                 const QWidget *widget) const
{
    painter->setRenderHint( QPainter::Antialiasing,          true );
    painter->setRenderHint( QPainter::TextAntialiasing,      true );
    painter->setRenderHint( QPainter::SmoothPixmapTransform, true );

    switch (element)
    {
        // QPushButton
        case CE_PushButton:
        {
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                if (!button->icon.isNull())
                {
                    QIcon::Mode  mode  = QIcon::Normal;
                    QIcon::State state = QIcon::Off;

                    if       (!(button->state & State_Enabled))  mode = QIcon::Disabled;
                    else if    (button->state & State_MouseOver) mode = QIcon::Active;

                    if          (button->state & State_On)       state = QIcon::On;

                    QPixmap pixmap = button->icon.pixmap(button->iconSize, mode, state);

                    painter->drawPixmap(button->rect, pixmap);
                    break;
                }
            }
            QProxyStyle::drawControl(element, option, painter, widget);
            break;
        }

        // QScrollBar
        case CE_ScrollBarAddPage:
        case CE_ScrollBarSubPage:
        {
            // polish() is currently forcing the widget to use our palette, but this
            // is one of the things the widgets like to override on us if given the
            // opportunity, so don't allow them to
            painter->fillRect(option->rect, m_standardPalette.color(QPalette::Window));
            break;
        }

        case CE_ScrollBarSubLine:
        case CE_ScrollBarAddLine:
        {
            PrimitiveElement arrow;
            if (option->state & State_Horizontal) {
                if (element == CE_ScrollBarAddLine)
                    arrow = option->direction == Qt::LeftToRight ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft;
                else
                    arrow = option->direction == Qt::LeftToRight ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
            } else {
                if (element == CE_ScrollBarAddLine)
                    arrow = PE_IndicatorArrowDown;
                else
                    arrow = PE_IndicatorArrowUp;
            }
            proxy()->drawPrimitive(arrow, option, painter, widget);
            break;
        }

        case CE_ScrollBarSlider:
        {
            // No primitives defined for this one.
            QPixmap pix;
            int state = 0; // normal

            if      (!(option->state & State_Enabled))   state = 3; // disabled
            else if   (option->state & State_MouseOver)  state = 1; // mouseover
            else if   (option->state & State_Sunken)     state = 2; // depressed

            if (option->state & State_Horizontal)
            {
                // Sub-optimal code: Wizardry 8 UI doesn't use horizontal
                // scrollbars, and this only done for completeness.
                // The shadowing on the control is wrong when we do this.
                QMatrix rot;
                rot.rotate(90);
                QImage hslider = m_sbslider->getImage( state ).transformed( QTransform(rot) );
                pix = QPixmap::fromImage( hslider );
            }
            else
            {
                pix = QPixmap::fromImage( m_sbslider->getImage( state ) );
            }

            painter->drawPixmap(option->rect, pix);
            break;
        }

        // QSlider
// We know we're abusing the enum type here, so shutup the compiler warning about it
#pragma GCC diagnostic ignored "-Wswitch"
        case CE_Slider:
        {
            // No primitives defined for this one.
            QPixmap pix;
            int state = 1; // normal

            if      (!(option->state & State_Enabled))   state = 3; // disabled
            else if   (option->state & State_MouseOver)  state = 2; // mouseover
            else if   (option->state & State_Sunken)     state = 2; // depressed

            pix = QPixmap::fromImage( m_slider->getImage( state ) );
            painter->drawPixmap(option->rect, pix);
            break;
        }
#pragma GCC diagnostic pop

        // QListView
        case CE_ItemViewItem:
        {
            if (const QStyleOptionViewItem *view = qstyleoption_cast<const QStyleOptionViewItem *>(option))
            {
                QStyleOptionViewItem  mod = *view;

                if (const WListWidget *w = qobject_cast<const WListWidget *>(widget))
                {
                    if (w->useTextColorInsteadOfCheckmarks())
                    {
                        // don't want the normal highlight - just want items to be either
                        // checked or unchecked, and instead of checkmarks we're going to
                        // do it with colour: white for enabled, grey for not

                        // Turn the alternate mode turned on so that QPalette::Highlight
                        // doesn't render. Also turn off the State_Selected.
                        mod.features |= QStyleOptionViewItem::Alternate;
                        mod.state    &= ~QStyle::State_Selected;

                        if (mod.checkState == Qt::Checked)
                        {
                            mod.palette.setBrush(QPalette::Normal,   QPalette::Text, w->m_checkedColor);
                            mod.palette.setBrush(QPalette::Inactive, QPalette::Text, w->m_checkedColor);
                            mod.palette.setBrush(QPalette::Disabled, QPalette::Text, w->m_checkedColor);
                        }
                        else if (mod.checkState == Qt::Unchecked)
                        {
                            mod.palette.setBrush(QPalette::Normal,   QPalette::Text, w->m_uncheckedColor);
                            mod.palette.setBrush(QPalette::Inactive, QPalette::Text, w->m_uncheckedColor);
                            mod.palette.setBrush(QPalette::Disabled, QPalette::Text, w->m_uncheckedColor);
                        }
                        // Turn off the checkState so it doesn't render an actual check as well
                        mod.features &= ~QStyleOptionViewItem::HasCheckIndicator;
                    }
                }

                if (view->text.contains( '\t' ) &&
                    (view->displayAlignment == Qt::AlignRight) &&
                   !(view->state & QStyle::State_Editing))
                {
                    // Yes we assume there is only ever one TAB - if there's
                    // more than that it won't render right

                    // Align the text before the TAB to the LHS of the row
                    mod.text = view->text.section('\t', 0, 0 );
                    mod.displayAlignment = Qt::AlignLeft;

                    QProxyStyle::drawControl(element, &mod, painter, widget);

                    // Align the text after the TAB to the RHS of the row
                    mod.text = view->text.section('\t', 1, 1 );
                    mod.displayAlignment = Qt::AlignRight;
                    mod.features &= ~QStyleOptionViewItem::HasCheckIndicator;
                    // We use this as a way to turn off the rendering of the background
                    // when the PE_PanelItemViewItem primitive is rendered by the
                    // drawControl() routine
                    mod.features |= QStyleOptionViewItem::Alternate;
                }
                QProxyStyle::drawControl(element, &mod, painter, widget);
            }
            else
            {
                QProxyStyle::drawControl(element, option, painter, widget);
            }
            break;
        }

        default:
            QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void Wizardry8Style::drawComplexControl(ComplexControl cc,
                                        const QStyleOptionComplex *opt,
                                        QPainter *p,
                                        const QWidget *widget) const
{
    switch (cc)
    {
        case CC_Slider:
        {
            if (const QStyleOptionSlider *sl = qstyleoption_cast<const QStyleOptionSlider *>(opt))
            {
                QRect groove   = proxy()->subControlRect(CC_Slider, sl, SC_SliderGroove, widget);
                QRect handle   = proxy()->subControlRect(CC_Slider, sl, SC_SliderHandle, widget);

                if ((sl->subControls & SC_SliderGroove) && groove.isValid())
                {
                    double scale = getParentScale(widget);

                    p->setPen( QColor(0xa0, 0x9a, 0x80));
                    p->drawRoundedRect(groove, 4*scale, 4*scale );

                    p->setPen( QColor(0x3e, 0x39, 0x1e)); // Supposed to be RGB:322e18 but too hard to see
                    QRectF r = QRectF( groove ).marginsRemoved( QMarginsF(2*scale, 2*scale, 2*scale, 2*scale) );
                    p->drawRoundedRect(r, 3*scale, 3*scale );

                    r = r.marginsRemoved( QMarginsF(2*scale, 2*scale, 2*scale, 2*scale) );
                    if (sl->orientation == Qt::Horizontal)
                    {
                        if (handle.x() > r.x())
                            r.setWidth( handle.x() - r.x() );
                        else
                            r.setWidth( 0 );
                    }
                    else
                    {
                        if (handle.y() > r.y())
                            r.setHeight( handle.y() - r.y() );
                        else
                            r.setHeight( 0 );
                    }
                    // No fillRoundedRect() method, so use a path
                    QPainterPath path;
                    path.addRoundedRect(r, 2*scale, 2*scale);
                    p->fillPath( path, QColor(0x00, 0xff, 0x00) );

                    if (sl->orientation == Qt::Horizontal)
                    {
                        if (r.width() > 2*scale)
                        {
                            r.adjust( r.width() - 2*scale, 0, 0, 0 );
                            p->fillRect( r, QColor(0x00, 0xff, 0x00) );
                        }
                    }
                    else
                    {
                        if (r.height() > 2*scale)
                        {
                            r.adjust( 0, r.height() - 2*scale, 0, 0 );
                            p->fillRect( r, QColor(0x00, 0xff, 0x00) );
                        }
                    }
                }

                if (sl->subControls & SC_SliderTickmarks)
                {
                    QStyleOptionSlider tmpSlider = *sl;
                    tmpSlider.subControls = SC_SliderTickmarks;
                    QCommonStyle::drawComplexControl(cc, &tmpSlider, p, widget);
                }

                if (sl->subControls & SC_SliderHandle)
                {
                    QStyleOptionSlider tmpSlider = *sl;
                    tmpSlider.rect = handle;
                    proxy()->drawControl(CE_Slider, &tmpSlider, p, widget);
                }
            }
            break;
        }

        case CC_SpinBox:
        {
            if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt))
            {
                if (sb->subControls & SC_SpinBoxUp)
                {
                    QStyleOptionSpinBox copy = *sb;

                    copy.subControls = SC_SpinBoxUp;

                    if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled))
                    {
                        copy.state &= ~State_Enabled;
                    }

                    if (sb->activeSubControls == copy.subControls && (sb->state & State_Sunken))
                    {
                        copy.state |= State_On;
                        copy.state |= State_Sunken;
                    }
                    else
                    {
                        copy.state |= State_Raised;
                        copy.state &= ~State_Sunken;
                    }

                    copy.state &= ~State_MouseOver;
                    if (sb->activeSubControls == copy.subControls)
                    {
                        copy.state |= State_MouseOver;
                    }

                    copy.rect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
                    proxy()->drawPrimitive(PE_IndicatorSpinPlus, &copy, p, widget);
                }

                if (sb->subControls & SC_SpinBoxDown)
                {
                    QStyleOptionSpinBox copy = *sb;

                    copy.subControls = SC_SpinBoxDown;

                    if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled))
                    {
                        copy.state &= ~State_Enabled;
                    }

                    if (sb->activeSubControls == copy.subControls && (sb->state & State_Sunken))
                    {
                        copy.state |= State_On;
                        copy.state |= State_Sunken;
                    }
                    else
                    {
                        copy.state |= State_Raised;
                        copy.state &= ~State_Sunken;
                    }

                    copy.state &= ~State_MouseOver;
                    if (sb->activeSubControls == copy.subControls)
                    {
                        copy.state |= State_MouseOver;
                    }

                    copy.rect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);
                    proxy()->drawPrimitive(PE_IndicatorSpinMinus, &copy, p, widget);
                }
            }
            break;
        }

        default:
            QProxyStyle::drawComplexControl(cc, opt, p, widget);
    }
}

int Wizardry8Style::styleHint(StyleHint hint, const QStyleOption *option,
                                  const QWidget *widget,
                                  QStyleHintReturn *returnData) const
{
    switch (hint)
    {
        case SH_DitherDisabledText:
            return 0;

        case SH_EtchDisabledText:
            return 1;

        // QSlider
        case SH_Slider_AbsoluteSetButtons:
            // make the slider jump to the position clicked instead of stepping
            return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);

        default:
            return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}
