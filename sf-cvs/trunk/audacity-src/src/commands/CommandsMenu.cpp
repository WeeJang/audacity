/**********************************************************************

  Audacity: A Digital Audio Editor

  CommandsMenu.cpp

  Brian Gunlogson

  This class creates menus, but does not assign them to anything.

**********************************************************************/

#include <wx/tokenzr.h>
#include <wx/log.h>
#include <wx/textctrl.h> // Needed for listctrl.h to include OK.
#include <wx/listctrl.h>

#include "CommandsMenu.h"


// On wxGTK, there may be many many many plugins, but the menus don't automatically
// allow for scrolling, so we build sub-menus.  If the menu gets longer than
// MAX_MENU_LEN, we put things in submenus that have MAX_SUBMENU_LEN items in them.
// 
#ifdef __WXGTK__
#define MAX_MENU_LEN 20
#define MAX_SUBMENU_LEN 15
#else
#define MAX_MENU_LEN 1000
#define MAX_SUBMENU_LEN 1000
#endif

///
///  Standard Constructor
///
CommandsMenu::CommandsMenu() : mCurrentID(0), mCurrentMenu(NULL)
{
}

///
///  Class Destructor.  Includes PurgeData, which removes
///  menubars
CommandsMenu::~CommandsMenu()
{
   //WARNING: This removes menubars that could still be assigned to windows!
   PurgeData();
}

///
/// Cleans up menubars that are wx arrays
///
void CommandsMenu::PurgeData()
{
   //delete the menubars
   for(size_t i = 0; i < mMenuBarList.GetCount(); i++)
      delete mMenuBarList[i]->menubar;

   //clear the arrays
   WX_CLEAR_ARRAY(mMenuBarList);
   mMenuBarList.Clear();

   WX_CLEAR_ARRAY(mSubMenuList);
   mSubMenuList.Clear();

   WX_CLEAR_ARRAY(mIdentifierNameList);
   mIdentifierNameList.Clear();

   //reset other variables
   mCurrentMenu = NULL;
   mCurrentID = 0;
}



///
/// Makes a new menubar for placement on the top of a project
/// Names it according to the passed-in string argument.
void CommandsMenu::AddMenuBar(wxString sMenu)
{
   MenuBarListEntry *tmpEntry = new MenuBarListEntry;

   tmpEntry->menubar = new wxMenuBar();
   tmpEntry->name = sMenu;

   mMenuBarList.Add(tmpEntry);
}


///
/// Retrieves the menubar based on the name given in AddMenuBar(name) 
///
wxMenuBar * CommandsMenu::GetMenuBar(wxString sMenu)
{
   for(unsigned int i = 0; i < mMenuBarList.GetCount(); i++)
   {
      if(!mMenuBarList[i]->name.Cmp(sMenu))
         return mMenuBarList[i]->menubar;
   }

   return NULL;
}


///
/// Retrieve the 'current' menubar; either NULL or the
/// last on in the mMenuBarList.
wxMenuBar * CommandsMenu::CurrentMenuBar()
{
   if(mMenuBarList.IsEmpty())
      return NULL;

   return mMenuBarList[mMenuBarList.GetCount()-1]->menubar;
}


///
/// This makes a new menu and adds it to the 'CurrentMenuBar'
///
void CommandsMenu::BeginMenu(wxString tName)
{
   wxMenu *tmpMenu = new wxMenu();

   mCurrentMenu = tmpMenu;

   CurrentMenuBar()->Append(mCurrentMenu, tName);
}


///
/// This ends a menu by setting the pointer to it
/// to NULL.  It is still attached to the CurrentMenuBar()
void CommandsMenu::EndMenu()
{
   mCurrentMenu = NULL;
}


///
/// This starts a new submenu, and names it according to
/// the function's argument.
void CommandsMenu::BeginSubMenu(wxString tName)
{
   SubMenuListEntry *tmpEntry = new SubMenuListEntry;

   tmpEntry->menu = new wxMenu();
   tmpEntry->name = tName;

   mSubMenuList.Add(tmpEntry);
}


///
/// This function is called after the final item of a SUBmenu is added.
/// Submenu items are added just like regular menu items; they just happen
/// after BeginSubMenu() is called but before EndSubMenu() is called.
void CommandsMenu::EndSubMenu()
{
   size_t submenu_count = mSubMenuList.GetCount()-1;

   //Save the submenu's information
   SubMenuListEntry *tmpSubMenu = mSubMenuList[submenu_count];

   //Pop off the new submenu so CurrentMenu returns the parent of the submenu
   mSubMenuList.RemoveAt(submenu_count);

   //Add the submenu to the current menu
   CurrentMenu()->Append(GetUniqueIdentifier(), tmpSubMenu->name, tmpSubMenu->menu, tmpSubMenu->name);

   delete tmpSubMenu;
}

///
/// This returns the 'Current' Submenu, which is the one at the
///  end of the mSubMenuList (or NULL, if it doesn't exist).
wxMenu * CommandsMenu::CurrentSubMenu()
{
   if(mSubMenuList.IsEmpty())
      return NULL;

   return mSubMenuList[mSubMenuList.GetCount()-1]->menu;
}

wxMenu * CommandsMenu::CurrentMenu()
{
   if(!mCurrentMenu)
      return NULL;

   wxMenu * tmpCurrentSubMenu = CurrentSubMenu();

   if(!tmpCurrentSubMenu)
   {
      return mCurrentMenu;
   }

   return tmpCurrentSubMenu;
}

void CommandsMenu::AddItem(wxString tName, wxString sFunctions, wxString sKeys)
{
   wxString finalName = AppendComboString(tName, sKeys);

   CurrentMenu()->Append(GetUniqueIdentifier(sFunctions, sKeys), finalName);
}

void CommandsMenu::AddSeparator()
{
   CurrentMenu()->AppendSeparator();
}

void CommandsMenu::AddDynamicItem(wxString sName)
{
   int effType = -1;
   EffectArray *effs;

   //determine what dynamic item to add
   if(!sName.Cmp("GeneratePlugins"))
      effType = INSERT_EFFECT;
   else if(!sName.Cmp("EffectPlugins"))
      effType = PROCESS_EFFECT;
   else if(!sName.Cmp("AnalyzePlugins"))
      effType = ANALYZE_EFFECT;
   else
      return;

   effs = Effect::GetEffects(BUILTIN_EFFECT | effType);
   AppendEffects(effs, sName, false);
   delete effs;

   effs = Effect::GetEffects(PLUGIN_EFFECT | effType);
   if (effs->GetCount() > 0)
   {
      AddSeparator();
      AppendEffects(effs, sName, true);
   }
   delete effs;

   if(!CurrentMenu()->GetMenuItemCount())
   {
      //Add disabled None item
      AddItem(_("None"), "", "");
      CurrentMenu()->Enable(mCurrentID, false);
   }
}

void CommandsMenu::AppendEffect(int idEffect, wxString sName, wxString sType)
{
   AddItem(sName, wxString::Format("%i@%s@Effect", idEffect, sType.c_str()), "");
}

void CommandsMenu::AppendEffects(EffectArray *effs, wxString sType, bool spill)
{
   unsigned int currentLen = CurrentMenu()->GetMenuItemCount();
   unsigned int effLen = effs->GetCount();
   unsigned int i;

   if (!spill || currentLen + effLen <= MAX_MENU_LEN)
   {
      for(i=0; i<effLen; i++)
         AppendEffect((*effs)[i]->GetID(), (*effs)[i]->GetEffectName(), sType);
      return;
   }

   // There were too many effects in this menu.  Put the
   // extras (plug-ins) in submenus.

   wxString label;
   int listnum = 1;
   int tmpmax = MAX_SUBMENU_LEN  < effLen? MAX_SUBMENU_LEN: effLen;


   //The first submenu starts at 1.
   BeginSubMenu(wxString::Format(_("Plugins 1 to %i"), tmpmax));

   for (i=0; i<effLen; i++) {
      AppendEffect((*effs)[i]->GetID(), (*effs)[i]->GetEffectName(), sType);
      
      if(((i+1) % MAX_SUBMENU_LEN) == 0 && i != (effLen - 1)){
         
         EndSubMenu();
         listnum++;
         
         tmpmax = i + MAX_SUBMENU_LEN  < effLen? 1 + i + MAX_SUBMENU_LEN: effLen;
         //This label the plugins by number in the submenu title (1 to 15, 15 to 30, etc.)
         BeginSubMenu(wxString::Format(_("Plugins %i to %i"),i+2,tmpmax));
      }
   }

   EndSubMenu();
}

///Gets an unused identifier, currently used for menus only
///WARNING: Does this conflict with the identifiers set for controls/windows?
///If it does, a workarround may be to keep controls below wxID_LOWEST
///and keep menus above wxID_HIGHEST
int CommandsMenu::GetUniqueIdentifier(wxString sFunctions, wxString sKeys)
{
   mCurrentID++;

   //Skip the reserved identifiers used by wxWindows
   if((mCurrentID >= wxID_LOWEST) && (mCurrentID <= wxID_HIGHEST))
      mCurrentID = wxID_HIGHEST+1;

   if(sFunctions.Length())
      SetIdentifierData(mCurrentID, sFunctions, sKeys);

   return mCurrentID;
}

void CommandsMenu::SetIdentifierData(int nID, wxString sFunctions, wxString sKeys)
{
   IdentifierNameListEntry *tmpEntry = new IdentifierNameListEntry;

   tmpEntry->id = nID;
   tmpEntry->functions = sFunctions;
   tmpEntry->keys = sKeys;
   tmpEntry->menu = CurrentMenu();

   mIdentifierNameList.Add(tmpEntry);
}

wxMenu * CommandsMenu::GetMenuFromIdentifier(int nID)
{
   for(unsigned int i = 0; i < mIdentifierNameList.GetCount(); i++)
   {
      if(mIdentifierNameList[i]->id == nID)
         return mIdentifierNameList[i]->menu;
   }

   return NULL;
}

int CommandsMenu::GetIdentifiersFromFunction(wxString sFunction, bool bReset)
{
   static unsigned int uIndex = 0;

   if(bReset)
      uIndex = 0;

   for(unsigned int i = uIndex; i < mIdentifierNameList.GetCount(); i++)
   {
      uIndex = i+1;

      wxStringTokenizer tFunctions(mIdentifierNameList[i]->functions, ":");
      while(tFunctions.HasMoreTokens())
      {
         wxString token = tFunctions.GetNextToken();

         if(!token.Cmp(sFunction))
            return mIdentifierNameList[i]->id;
      }
   }

   return -1;
}

int CommandsMenu::GetIdentifierFromFunctions(wxString sFunctions)
{
   for(unsigned int i = 0; i < mIdentifierNameList.GetCount(); i++)
   {
      if(!mIdentifierNameList[i]->functions.Cmp(sFunctions))
         return mIdentifierNameList[i]->id;
   }

   return -1;
}

wxString CommandsMenu::GetFunctionsFromIdentifier(int nID)
{
   for(unsigned int i = 0; i < mIdentifierNameList.GetCount(); i++)
   {
      if(mIdentifierNameList[i]->id == nID)
         return mIdentifierNameList[i]->functions;
   }

   return "";
}

wxString CommandsMenu::GetKeysFromIdentifier(int nID)
{
   for(unsigned int i = 0; i < mIdentifierNameList.GetCount(); i++)
   {
      if(mIdentifierNameList[i]->id == nID)
         return mIdentifierNameList[i]->keys;
   }

   return "";
}

int CommandsMenu::GetIdentifierFromKey(wxString sKey)
{
   for(unsigned int i = 0; i < mIdentifierNameList.GetCount(); i++)
   {
      wxStringTokenizer tKeys(mIdentifierNameList[i]->keys, ":");
      while(tKeys.HasMoreTokens())
      {
         wxString token = tKeys.GetNextToken();

         if(!token.Cmp(sKey))
            return mIdentifierNameList[i]->id;
      }
   }

   return -1;
}

int CommandsMenu::GetIdentifierFromKeys(wxString sKeys)
{
   return GetIdentifierFromKey(GetFirstKey(sKeys));
}

wxString CommandsMenu::GetFirstKey(wxString sKeys)
{
   wxStringTokenizer tKeys(sKeys, ":");
   if(tKeys.HasMoreTokens())
   {
      return tKeys.GetNextToken();
   }

   return "";
}

wxString CommandsMenu::AppendComboString(wxString tName, wxString sKeys)
{
   //see if tName contains tabs
   if((tName.Find('\t') != -1) || (tName.Find("\\t") != -1))
      return tName;

   wxString firstKey = GetFirstKey(sKeys);

   if(!firstKey)
      return tName;

   return (tName+"\t"+firstKey);
}

/// FillKeyBindingsList populates a wxListCtrl with 
/// The key binding information, e.g. Ctrl-O == 'Open'.
/// It would be better design if there was a CommandsMenuIterator
/// for then CommandsMenu wouldn't need to inculude <listctrl.h>
void CommandsMenu::FillKeyBindingsList( wxListCtrl * pList )
{
   int j=0;
   for(unsigned int i = 0; i < mIdentifierNameList.GetCount(); i++)
   {
      IdentifierNameListEntry * pEntry = mIdentifierNameList[i];

      if( !pEntry->keys.IsEmpty() )
      {
         // We get the function name but don't currently use it.
         wxString label = pEntry->menu->GetLabel( pEntry->id );
         wxString key   = pEntry->keys;
         wxString func  = pEntry->functions;

         // Non-standard use of the StartsWith function to 
         // remove a prefix of 'On' if it is present.
         func.StartsWith( "On", &func );

         // The menu label is in the currently selected language.
         // We'd like to use it, but it has extra 'stuff' in it.
         // so we remove that stuff.
         label.Replace("&","");
         label.Replace("...","\t");
         int descriptionLength =label.First("\t");
         if( descriptionLength > 0)
            label = label.Truncate( descriptionLength );

#if 0
         wxLogDebug("Key: %s Value: %s Label: %s", 
            key,
            func,
            label
            );
#endif
         pList->InsertItem( j, _T("") );
         pList->SetItem( j, 1, key );
         pList->SetItem( j, 2, label );
         j++;
      }
   }
}
