#include "QURibbonBar.h"
#include "QULogService.h"

#include "QUMonty.h"

#include <QToolButton>
#include <QIcon>
#include <QHBoxLayout>
#include <QMenu>
#include <QSettings>
#include <QDir>

QURibbonBar::QURibbonBar(QWidget *parent): QTabWidget(parent), _menuHidden(false) {
	setupUi(this);

	tabBar()->installEventFilter(this);

	helpBtn = new QToolButton(this);
	helpBtn->setAutoRaise(true);
    helpBtn->setIcon(QIcon(":/icons/help.png"));
	helpBtn->setToolTip(tr("Show Monty now to help you."));

	montyBtn = new QToolButton(this);
	montyBtn->setAutoRaise(true);
	montyBtn->setCheckable(true);
    montyBtn->setToolTip(tr("Allow Monty to help you on his own."));

	montyBtn->setIcon(QIcon(":/monty/normal.png"));

	QWidget *corner = new QWidget(this);
	corner->setLayout(new QHBoxLayout(this));
	corner->layout()->addWidget(montyBtn);
	corner->layout()->addWidget(helpBtn);
    corner->layout()->setContentsMargins(0, 0, 0, 0);
	corner->layout()->setSpacing(0);

	setCornerWidget(corner, Qt::TopRightCorner);

	hideBtn = new QToolButton(this);
	hideBtn->setAutoRaise(true);
	hideBtn->setIconSize(QSize(10, 12));
	//MB: hide hideBtn as long as hiding the ribbon menu isn't working
	hideBtn->setVisible(false);
	connect(hideBtn, SIGNAL(clicked()), this, SLOT(toggleMenuHidden()));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(changeCurrentTab(int)));

	setCornerWidget(hideBtn, Qt::TopLeftCorner);

	// recover menu state
	QSettings s;
    setMenuHidden(s.value("hideRibbonBar", false).toBool());
}

void QURibbonBar::setShortcut(QToolButton *w, const QKeySequence &key) {
	QAction *a = new QAction(this);
	a->setShortcut(key);
	connect(a, SIGNAL(triggered()), w, SLOT(click()));
	addAction(a);

	w->setToolTip(QString("%1 (%2)").arg(w->toolTip()).arg(key.toString()));
}

QSize QURibbonBar::sizeHint() const {
	if(menuHidden())
		return tabBar()->sizeHint();// + QSize(0, 5);
	else
		return QTabWidget::sizeHint();
}

void QURibbonBar::setMenuHidden(bool hide) {
	QSettings s;
	s.setValue("hideRibbonBar", hide);

	_menuHidden = hide;

	if(menuHidden()) {
		useHiddenStyle();
		hideBtn->setIcon(QIcon(":/bullets/bullet_add.png"));
		hideBtn->setToolTip(tr("Show full menu."));
	} else {
		useVisibleStyle();
		hideBtn->setIcon(QIcon(":/bullets/bullet_remove.png"));
		hideBtn->setToolTip(tr("Show small menu."));
	}

	setGeometry(x(), y(), width(), sizeHint().height());
}

void QURibbonBar::toggleMenuHidden() {
	setMenuHidden(!menuHidden());
}

void QURibbonBar::useVisibleStyle() {
	QStringList styles;

	styles << "QTabWidget::pane {border-bottom: 1px solid #C2C7CB; border-top: 1px solid #C2C7CB; margin-bottom: 3px; background-color: white}";
	styles << "QTabBar::tab {background: transparent; padding: 0.2em 0.9em; color: #333333; border: 1px solid transparent}";
//	styles << "QTabWidget::tab-bar {left: 5px}";
	styles << "QTabBar::tab:selected {color: black; background: white; border-top-left-radius: 4px; border-top-right-radius: 4px; border: 1px solid #C4C4C3; border-bottom-color: white}";
	styles << "QGroupBox {padding: 2px 3px 6px 3px; margin: 0 2px 4px 2px; border: 0; font-size: 7pt; border-bottom: 1px solid #C2C7CB; border-left: 1px solid #C2C7CB; border-right: 1px solid #C2C7CB; border-bottom-left-radius: 4px; border-bottom-right-radius: 4px; color: #C2C7CB}";
	styles << "QGroupBox::title {subcontrol-position: bottom center; subcontrol-origin: margin}";

	setStyleSheet(styles.join(" "));
}

void QURibbonBar::useHiddenStyle() {
	QStringList styles;

	styles << "QTabWidget::pane {border-bottom: 1px solid transparent; border-top: 1px solid transparent; margin-bottom: 3px; background-color: transparent}";
	styles << "QGroupBox {padding: 2px 3px 6px 3px; margin: 0 2px 4px 2px; border: 0; font-size: 7pt; border-bottom: 1px solid #C2C7CB; border-left: 1px solid #C2C7CB; border-right: 1px solid #C2C7CB; border-bottom-left-radius: 4px; border-bottom-right-radius: 4px; color: #C2C7CB}";
	styles << "QGroupBox::title {subcontrol-position: bottom center; subcontrol-origin: margin}";

	styles << "QTabBar::tab {background: transparent; padding: 0.2em 0.9em; color: #333333; border: 1px solid transparent}";
//	styles << "QTabWidget::tab-bar {left: 5px}";
	styles << "QTabBar::tab:selected {background: transparent; color: #333333; border: 1px solid transparent; border-bottom-style: none}";
//	styles << "QTabBar::tab:selected {border-top-left-radius: 4px; border-top-right-radius: 4px; border: 1px solid #C4C4C3; }";
	styles << "QTabBar::tab:hover {color: black; border-radius: 4px; background: white; border: 1px solid #C4C4C3};";

	setStyleSheet(styles.join(" "));
}

void QURibbonBar::changeCurrentTab(int) {
	setMenuHidden(false);
}

bool QURibbonBar::eventFilter(QObject *target, QEvent *event) {
	if(target == tabBar()) {
		if(event->type() == QEvent::MouseButtonPress) {
			setMenuHidden(false);
		}
	}
	return QTabWidget::eventFilter(target, event);
}
