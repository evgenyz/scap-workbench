/*
 * Copyright 2013 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Martin Preisler <mpreisle@redhat.com>
 */

#ifndef SCAP_WORKBENCH_TAILORING_WINDOW_H_
#define SCAP_WORKBENCH_TAILORING_WINDOW_H_

#include "ForwardDecls.h"

#include <QDialog>
#include <QUndoCommand>
#include <QUndoStack>

extern "C"
{
#include <xccdf_benchmark.h>
#include <xccdf_policy.h>
}

#include "ui_TailoringWindow.h"
#include "ui_ProfilePropertiesDockWidget.h"
#include "ui_XCCDFItemPropertiesDockWidget.h"

class TailoringWindow;

/**
 * @brief Displays profile properties and allows editing of profile title
 */
class ProfilePropertiesDockWidget : public QDockWidget
{
    Q_OBJECT

    public:
        ProfilePropertiesDockWidget(TailoringWindow* window, QWidget* parent = 0);
        virtual ~ProfilePropertiesDockWidget();

        /**
         * @brief Takes profile's current ID and title and sets both QLineEdit widgets accordingly
         */
        void refresh();

    protected slots:
        void profileTitleChanged(const QString& newTitle);
        void profileDescriptionChanged();

    protected:
        /// Prevents a redo command being created when actions are undone or redone
        bool mUndoRedoInProgress;

        /// UI designed in Qt Designer
        Ui_ProfilePropertiesDockWidget mUI;

        /// Owner TailoringWindow that provides profile for editing/viewing
        TailoringWindow* mWindow;
};

/**
 * @brief Provides reference about currently selected XCCDF item
 */
class XCCDFItemPropertiesDockWidget : public QDockWidget
{
    Q_OBJECT

    public:
        XCCDFItemPropertiesDockWidget(QWidget* parent = 0);
        virtual ~XCCDFItemPropertiesDockWidget();

        /**
         * @brief Changes currently inspected XCCDF item
         *
         * @note This method automatically calls refresh to load new data
         */
        void setXccdfItem(struct xccdf_item* item);

        /**
         * @brief Loads properties from currently set XCCDF items and sets widgets accordingly
         */
        void refresh();

    protected:
        /// UI designed in Qt Designer
        Ui_XCCDFItemPropertiesDockWidget mUI;

        /// Currently inspected XCCDF item
        struct xccdf_item* mXccdfItem;
};

/**
 * @brief Stores info about one selection or deselection of an XCCDF item
 */
class XCCDFItemSelectUndoCommand : public QUndoCommand
{
    public:
        XCCDFItemSelectUndoCommand(TailoringWindow* window, QTreeWidgetItem* item, bool newSelect);
        virtual ~XCCDFItemSelectUndoCommand();

        virtual int id() const;

        virtual void redo();
        virtual void undo();

    private:
        TailoringWindow* mWindow;

        QTreeWidgetItem* mTreeItem;
        /// selection state after this undo command is "redone" (applied)
        bool mNewSelect;
};

/**
 * @brief Stores XCCDF profile title change undo info
 */
class ProfileTitleChangeUndoCommand : public QUndoCommand
{
    public:
        ProfileTitleChangeUndoCommand(TailoringWindow* window, const QString& oldTitle, const QString& newTitle);
        virtual ~ProfileTitleChangeUndoCommand();

        virtual int id() const;

        virtual void redo();
        virtual void undo();

        virtual bool mergeWith(const QUndoCommand *other);

    private:
        TailoringWindow* mWindow;

        QString mOldTitle;
        QString mNewTitle;
};

/**
 * @brief Stores XCCDF profile description change undo info
 */
class ProfileDescriptionChangeUndoCommand : public QUndoCommand
{
    public:
        ProfileDescriptionChangeUndoCommand(TailoringWindow* window, const QString& oldDesc, const QString& newDesc);
        virtual ~ProfileDescriptionChangeUndoCommand();

        virtual int id() const;

        virtual void redo();
        virtual void undo();

        virtual bool mergeWith(const QUndoCommand *other);

    private:
        TailoringWindow* mWindow;

        QString mOldDesc;
        QString mNewDesc;
};
/**
 * @brief Tailors given profile by editing it directly
 *
 * If you want to inherit a profile and tailor that, create a new profile,
 * set up the inheritance and then pass the new profile to this class.
 */
class TailoringWindow : public QMainWindow
{
    Q_OBJECT

    public:
        TailoringWindow(struct xccdf_policy* policy, struct xccdf_benchmark* benchmark, MainWindow* parent = 0);
        virtual ~TailoringWindow();

        /**
         * @brief Makes sure that given XCCDF item is selected or deselected in the policy and profile
         *
         * This method adds a new select to the policy and profile. This select overrides all
         * previous selects if any.
         */
        void setItemSelected(struct xccdf_item* xccdfItem, bool selected);

        /**
         * @brief Synchronizes given tree item to represent given XCCDF item
         *
         * @param recursive If true synchronization is called on children of the tree item and XCCDF item as well
         */
        void synchronizeTreeItem(QTreeWidgetItem* treeItem, struct xccdf_item* xccdfItem, bool recursive);

        /**
         * @brief Retrieves ID of profile that is being tailored (in suitable language)
         */
        QString getProfileID() const;

        /**
         * @brief Goes through profile title texts and sets one of them to given title
         *
         * @see TailoringWindow::setProfileTitleWithUndoCommand
         */
        void setProfileTitle(const QString& title);

        /**
         * @brief Retrieves title of profile that is being tailoring (in suitable language)
         */
        QString getProfileTitle() const;

        /**
         * @brief Creates a new undo command that changes title of tailored profile and pushes it onto the undo stack
         *
         * @see TailoringWindow::setProfileTitle
         */
        void setProfileTitleWithUndoCommand(const QString& newTitle);

        /**
         * @brief Goes through profile description texts and sets one of them to given title
         *
         * @see TailoringWindow::setProfileDescriptionWithUndoCommand
         */
        void setProfileDescription(const QString& description);

        /**
         * @brief Retrieves description of profile that is being tailoring (in suitable language)
         */
        QString getProfileDescription() const;

        /**
         * @brief Creates a new undo command that changes description of tailored profile and pushes it onto the undo stack
         *
         * @see TailoringWindow::setProfileDescription
         */
        void setProfileDescriptionWithUndoCommand(const QString& newDescription);

        /**
         * @brief Refreshes profile properties dock widget to accurately represent tailored profile
         */
        void refreshProfileDockWidget();

    protected:
        /// Reimplemented to refresh profiles and selected rules in the parent main window
        virtual void closeEvent(QCloseEvent * event);

        MainWindow* mParentMainWindow;

        /// if > 0, ignore itemChanged signals, these would just excessively add selects and bloat memory
        unsigned int mSynchronizeItemLock;

        /// UI designed in Qt Designer
        Ui_TailoringWindow mUI;

        ProfilePropertiesDockWidget* mProfilePropertiesDockWidget;
        XCCDFItemPropertiesDockWidget* mItemPropertiesDockWidget;

        struct xccdf_policy* mPolicy;
        struct xccdf_profile* mProfile;
        struct xccdf_benchmark* mBenchmark;

        QUndoStack mUndoStack;

    protected slots:
        void itemSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
        void itemChanged(QTreeWidgetItem* item, int column);
};

#endif
