// -*-c++-*-

#ifndef GUI_H_
#define GUI_H_

#undef OK
#include <gtkmm.h>
#include <libglademm/xml.h>

#include <generic/apt/apt.h>
#include <generic/apt/aptitude_resolver_universe.h>
#include <generic/apt/resolver_manager.h>
#include <generic/problemresolver/solution.h>

#include <sigc++/slot.h>

namespace gui
{

  /**
   * This is a list of tab types.
   */
  enum TabType
  {
    Dashboard, Download, Packages, Info, Preview, Resolver, InstallRemove
  };

  /**
   * This is a list of packages actions.
   * TODO: This probably already exist. Find it.
   * FIXME: Description shouldn't be here.
   */
  enum PackagesAction
  {
    Install, Remove, Purge, Keep, Hold, Description
  };

  class guiOpProgress : public OpProgress
  { // must derive to read protected member..
    private:
      float sanitizePercentFraction(float percent);
    public:
      ~guiOpProgress();
      void Update();
  };

  /**
   * A Tab contains a widget and some metadata for inserting into the notebook.
   */
  class Tab
  {
    private:
      TabType type;
      Glib::ustring label;
      Glib::RefPtr<Gnome::Glade::Xml> xml;
      Gtk::Label * label_label;
      Gtk::Widget * label_widget;
      Gtk::Widget * widget;
    public:
      /** \brief Construct a new tab.
       *
       *  \param _type The type of the new tab.
       *  \param _label The label of the new tab.
       *  \param _xml  The XML object from which to take the widget
       *               of the new tab.
       *  \param widgetName  The name of the new tab's associated
       *                     widget within the given XML tree.
       */
      Tab(TabType _type, const Glib::ustring &_label,
	  const Glib::RefPtr<Gnome::Glade::Xml> &_xml, const std::string &widgetName);
      Glib::ustring get_label() { return label; }
      Gtk::Widget * get_label_widget() { return label_widget; }
      void set_label(Glib::ustring);
      TabType get_type() { return type; }
      Gtk::Widget * get_widget() const { return widget; }
      const Glib::RefPtr<Gnome::Glade::Xml> &get_xml() { return xml; }
  };

  class PackagesTab;
  class PackagesView;

  class DownloadColumns : public Gtk::TreeModel::ColumnRecord
  {
    public:
      Gtk::TreeModelColumn<Glib::ustring> URI;
      Gtk::TreeModelColumn<Glib::ustring> Description;
      Gtk::TreeModelColumn<Glib::ustring> ShortDesc;

      DownloadColumns();
  };

  class DownloadTab : public Tab
  {
    public:
      Glib::RefPtr<Gtk::ListStore> download_store;
      DownloadColumns download_columns;
      Gtk::TreeView * pDownloadTreeView;

      DownloadTab(const Glib::ustring &label);
      void createstore();
  };

  /**
   * The PackagesMarker marks packages belonging to a PackagesTab
   */
  class PackagesMarker
  {
    private:
      PackagesView * view;
      void dispatch(pkgCache::PkgIterator pkg, pkgCache::VerIterator ver, PackagesAction action);
      void callback(const Gtk::TreeModel::iterator& iter, PackagesAction action);
    public:
      /** \brief Construct a packages marker for tab.
       *
       *  \param tab The tab on which the marking takes place.
       */
      PackagesMarker(PackagesView * view);
      void select(PackagesAction action);
  };

  /**
   * The context menu for packages in PackagesTab
   */
  class PackagesContextMenu
  {
    private:
      Gtk::Menu * pMenu;
      Gtk::ImageMenuItem * pMenuInstall;
      Gtk::ImageMenuItem * pMenuRemove;
      Gtk::ImageMenuItem * pMenuPurge;
      Gtk::ImageMenuItem * pMenuKeep;
      Gtk::ImageMenuItem * pMenuHold;
    public:
      /** \brief Construct a context menu for tab.
       *
       *  \param tab The tab who owns the context menu.
       *  \param marker The marker to use to execute the actions.
       */
    PackagesContextMenu(PackagesView * view);
    Gtk::Menu * get_menu() const { return pMenu; };
  };

  class PackagesColumns : public Gtk::TreeModel::ColumnRecord
  {
    public:
      Gtk::TreeModelColumn<pkgCache::PkgIterator> PkgIterator;
      Gtk::TreeModelColumn<pkgCache::VerIterator> VerIterator;
      Gtk::TreeModelColumn<Glib::ustring> CurrentStatus;
      Gtk::TreeModelColumn<Glib::ustring> SelectedStatus;
      Gtk::TreeModelColumn<Glib::ustring> Name;
      Gtk::TreeModelColumn<Glib::ustring> Section;
      Gtk::TreeModelColumn<Glib::ustring> Version;

      PackagesColumns();
  };

  /** \brief Interface for generating tree-views.
   *
   *  A tree-view generator takes each package that appears in the
   *  current package view and places it into an encapsulated
   *  Gtk::TreeModel.
   */
  class PackagesTreeModelGenerator
  {
  public:
    virtual ~PackagesTreeModelGenerator();

    /** \brief Add the given package and version to this tree view.
     *
     *  \param pkg  The package to add.
     *  \param ver  The version of pkg to add, or an end
     *              iterator to add a versionless row.
     *
     *  \param reverse_package_store   A multimap in which a pair will
     *  be inserted for each row generated by this add().
     *
     *  \note Technically we could build reverse_package_store in a
     *  second pass instead of passing it here; I think this is
     *  cleaner and it might be worth giving it a try in the future.
     */
    virtual void add(const pkgCache::PkgIterator &pkg, const pkgCache::VerIterator &ver,
		     std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * reverse_package_store) = 0;

    /** \brief Perform actions that need to be taken after adding all
     *  the packages.
     *
     *  For instance, this typically sorts the entire view.
     */
    virtual void finish() = 0;

    /** \brief Retrieve the model associated with this generator.
     *
     *  The model will be filled in as add_package() is invoked.
     *  Normally you should only use the model once it is entirely
     *  filled in (to avoid unnecessary screen updates).
     *
     *  \return  The model built by this generator.
     */
    virtual Glib::RefPtr<Gtk::TreeModel> get_model() = 0;
  };

  class PackagesTreeView : public Gtk::TreeView
  {
    public:
      PackagesTreeView(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
      bool on_button_press_event(GdkEventButton* event);
      sigc::signal<void, GdkEventButton*> signal_context_menu;
      sigc::signal<void> signal_selection;
  };

  class PackagesView
  {
    public:
      typedef sigc::slot1<PackagesTreeModelGenerator *,
			  PackagesColumns *> GeneratorK;
    private:
      PackagesTreeView * treeview;
      PackagesColumns * packages_columns;
      Glib::RefPtr<Gtk::TreeModel> packages_store;
      std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * reverse_packages_store;
      PackagesContextMenu * context;
      PackagesMarker * marker;
      GeneratorK generatorK;

      /** \brief Build the tree model using the given generator.
       *
       *  This adds all packages that pass the current limit to the
       *  generator, one at a time.
       *
       *  \param  generatorK         A function that constructs a generator
       *                             to use in building the new store.
       *  \param  packages_columns   The columns of the new store.
       *  \param  reverse_packages_store  A multimap to be filled with the
       *                                  location of each package iterator
       *                                  in the generated store.
       *  \param  limit             The limit pattern for the current view.
       */
      static Glib::RefPtr<Gtk::TreeModel> build_store(const GeneratorK &generatorK,
						      PackagesColumns *packages_columns,
						      std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * reverse_packages_store,
						      Glib::ustring limit);
    public:
      /** \brief Construct a new packages view.
       *
       *  \param _generatorK A constructor for the callback
       *                     object used to build the model.
       *  \param refGlade    The XML tree containing
       *                     the widgets for this view.
       */
      PackagesView(const GeneratorK &_generatorK,
		   Glib::RefPtr<Gnome::Glade::Xml> refGlade);
      ~PackagesView();
      void context_menu_handler(GdkEventButton * event);
      void refresh_packages_view(std::set<pkgCache::PkgIterator> changed_packages);
      void relimit_packages_view(Glib::ustring limit);
      sigc::signal<void, pkgCache::PkgIterator, pkgCache::VerIterator> signal_on_package_selection;
      PackagesTreeView * get_treeview() { return treeview; };
      PackagesColumns * get_packages_columns() { return packages_columns; };
      PackagesMarker * get_marker() { return marker; };
      Glib::RefPtr<Gtk::TreeModel> get_packages_store() { return packages_store; };
      std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * get_reverse_packages_store() { return reverse_packages_store; };
  };

  class PackagesTab : public Tab
  {
    private:
      PackagesView * pPackagesView;
      Gtk::TextView * pPackagesTextView;
      Gtk::Entry * pLimitEntry;
      Gtk::Button * pLimitButton;
    public:
      PackagesTab(const Glib::ustring &label);
      Glib::RefPtr<Gtk::ListStore> create_store();
      std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * create_reverse_store();
      void repopulate_model();
      void display_desc(pkgCache::PkgIterator pkg, pkgCache::VerIterator ver);
      PackagesView * get_packages_view() { return pPackagesView; };
  };

  // TODO: This needs to share more code with PackagesTab.
  //       A PreviewTab is really a PackagesTab with a TreeStore.
  class PreviewTab : public Tab
  {
    private:
      PackagesView * pPackagesView;
      Gtk::TextView * pPackagesTextView;
      Gtk::Entry * pLimitEntry;
      Gtk::Button * pLimitButton;
    public:
      PreviewTab(const Glib::ustring &label);
      Glib::RefPtr<Gtk::TreeStore> create_store();
      std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * create_reverse_store();
      void repopulate_model();
      void display_desc(pkgCache::PkgIterator pkg, pkgCache::VerIterator ver);
      PackagesView * get_packages_view() { return pPackagesView; };
  };

  class ResolverColumns : public Gtk::TreeModel::ColumnRecord
  {
    public:
      Gtk::TreeModelColumn<pkgCache::PkgIterator> PkgIterator;
      Gtk::TreeModelColumn<pkgCache::VerIterator> VerIterator;
      Gtk::TreeModelColumn<Glib::ustring> Name;
      Gtk::TreeModelColumn<Glib::ustring> Action;

      ResolverColumns();
  };

  class ResolverView : public Gtk::TreeView
  {
    public:
      Glib::RefPtr<Gtk::TreeStore> resolver_store;
      ResolverColumns resolver_columns;

      ResolverView(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
      void createstore();
  };

  class ResolverTab : public Tab
  {
    private:
      typedef generic_solution<aptitude_universe> aptitude_solution;

      ResolverView * pResolverView;
      Gtk::Label * pResolverStatus;
      Gtk::Button * pResolverPrevious;
      Gtk::Button * pResolverNext;
      Gtk::Button * pResolverApply;

      aptitude_solution sol;
      resolver_manager::state state;

      std::string archives_text(const pkgCache::VerIterator &ver);
      std::string dep_targets(const pkgCache::DepIterator &start);
      std::wstring dep_text(const pkgCache::DepIterator &d);
      bool do_previous_solution_enabled();
      void do_previous_solution();
      bool do_next_solution_enabled();
      void do_next_solution();
      bool do_apply_solution_enabled_from_state(const resolver_manager::state &state);
      void do_apply_solution();
    public:
      ResolverTab(const Glib::ustring &label);
      Glib::RefPtr<Gtk::TreeStore> create_store();
      std::multimap<pkgCache::PkgIterator, Gtk::TreeModel::iterator> * create_reverse_store();
      void repopulate_model();
      ResolverView * get_packages_view() { return pResolverView; };
  };

  /**
   * This is a custom widget that handles placement of tabs
   */
  class TabsManager : public Gtk::Notebook
  {
    private:
      /**
       * Gives the position for the next tab of given type
       * @param type type of tab
       * @return position of the next tab of this type
       */
      int next_position(TabType type);
      /**
       * Gives the number of tabs of given type
       * @param type type of tab
       * @return number of tabs of this type
       */
      int number_of(TabType type);
    public:
      /**
       * Glade::Xml derived widget constructor.
       */
      TabsManager(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
      /**
       * Appends a tab to the notebook
       * @param tab tab to append
       * @return position of the appended tab
       */
      int append_page(Tab& tab);
      /**
       * Remove a tab from the notebook
       * @param tab tab to remove
       */
      void remove_page(Tab& tab);
  };

  /**
   * This is the main Aptitude custom window widget.
   */
  class AptitudeWindow : public Gtk::Window
  {
    private:
      Gtk::ToolButton * pToolButtonDashboard;
      Gtk::ToolButton * pToolButtonUpdate;
      Gtk::ToolButton * pToolButtonPackages;
      Gtk::ToolButton * pToolButtonPreview;
      Gtk::ToolButton * pToolButtonResolver;
      Gtk::ToolButton * pToolButtonInstallRemove;

      Gtk::ImageMenuItem * pMenuFilePackageRun;
      Gtk::ImageMenuItem * pMenuFileUpdateLists;
      Gtk::ImageMenuItem * pMenuFileMarkUpgradable;
      Gtk::ImageMenuItem * pMenuFileForgetNew;
      Gtk::ImageMenuItem * pMenuFileKeepAll;
      Gtk::ImageMenuItem * pMenuFileClean;
      Gtk::ImageMenuItem * pMenuFileAutoclean;
      Gtk::ImageMenuItem * pMenuFileReloadCache;
      Gtk::ImageMenuItem * pMenuFileSweep;
      Gtk::ImageMenuItem * pMenuFileSuToRoot;
      Gtk::ImageMenuItem * pMenuFileExit;

      Gtk::ProgressBar * pProgressBar;
      Gtk::Statusbar * pStatusBar;
      TabsManager * pNotebook;
    public:
      /**
       * Glade::Xml derived widget constructor.
       */
      AptitudeWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)/* : Gtk::Window(cobject)*/;

    Gtk::ProgressBar * get_progress_bar() const { return pProgressBar; }
    Gtk::Statusbar * get_status_bar() const { return pStatusBar; }
    TabsManager * get_notebook() const { return pNotebook; }
  };

  void main(int argc, char *argv[]);

}

#endif /*GUI_H_*/
