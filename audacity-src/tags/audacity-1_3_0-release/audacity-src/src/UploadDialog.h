/**********************************************************************

  Audacity: A Digital Audio Editor

  AboutDialog.h

  Mike Underwood

**********************************************************************/

#ifndef __AUDACITY_UPLOAD_DLG__
#define __AUDACITY_UPLOAD_DLG__

#include <wx/dialog.h>
#include <wx/protocol/ftp.h>
#include <wx/combobox.h>
#include <wx/textfile.h>

#define wxID_FTPNAME 1000
#define wxID_FTPHOST 1001
#define wxID_FTPUSER 1002
#define wxID_FTPPASS 1003
#define wxID_SAVEFTP 1004
#define wxID_USEPASV 1005
#define wxID_CONNECT 1006
#define wxID_DISCONNECT  1007
#define wxID_UPLOAD_FILE 1008
#define wxID_UPLOAD_DIR  1009
#define wxID_STATUSBAR   1010
#define wxID_SITELIST    1011
#define wxID_FILEMANAGER 1012
#define wxID_CREATEDIR   1013
#define wxID_NEWSITE     1014
#define wxID_DELSITE     1015

#define wxID_POPUP_DOWNLOAD 2000
#define wxID_POPUP_RENAME   2001
#define wxID_POPUP_DELETE   2002

#define CHUNKSIZE 128

class wxBoxSizer;
class wxButton;
class wxCheckBox;
class wxListView;
class wxTextCtrl;
class wxFTP;
class wxStatusBar;

class UploadDialog:public wxDialog {
   DECLARE_DYNAMIC_CLASS(UploadDialog)

 public:
   UploadDialog(wxWindow * parent);
   virtual ~ UploadDialog();

   void OnConnect(wxCommandEvent & event);
   void OnDisconnect(wxCommandEvent & event);
   void OnUploadFile(wxCommandEvent & event);
   void OnUploadDir(wxCommandEvent & event);
   void OnCreateDir(wxCommandEvent & event);
   void OnActivateItem (wxListEvent &event);
   void OnActivateSite (wxListEvent &event);
   void OnListRightClick (wxListEvent & event);
   void OnPopupMenu (wxCommandEvent & event);
   void OnNewSite (wxCommandEvent & event);
   void OnDeleteSite (wxCommandEvent &event);
   void OnFtpChange(wxCommandEvent & event);
   void OnSelectSite (wxListEvent &event);
   void OnCharPress (wxKeyEvent &event);

   void GetDirContents(void);
   void RefreshFiles(void);
   void ExtractListData (wxString string, wxArrayString *results);

   bool SaveFtpSite(wxString name, wxString host, wxString user, wxString pass);
   void LoadFtpSiteList (void);
   void UpdateSiteList (void);

   void DownloadItem (wxString &src, wxString &dest, bool dir, bool multi);
   void DownloadItem (wxString &src, bool dir);
   void DownloadMultipleItems(void);
   void DownloadFile (wxString src, wxString dest);
   void DownloadDir (wxString src, wxString dest);

   void GetDeleteList (wxString src);
   void RemoveItems(wxArrayString *files, wxArrayString *dirs);
   void UploadFile(wxString src, wxString dest);
   void UploadDir (wxString src, wxString dest);

   wxBoxSizer *topsizer;
   wxButton *btnConnect;
   wxButton *btnDisconnect;
   wxButton *btnUploadFile;
   wxButton *btnUploadDir;
   wxButton *btnCreateDir;

   wxButton *btnNewSite;
   wxButton *btnDelSite;

   wxCheckBox *cbSave;
   wxCheckBox *cbPasv;
   wxImageList *icons;   
   wxListView *siteList;
   wxListView *fileManager;
   wxTextCtrl *txtFtpName;
   wxTextCtrl *txtFtpHost;
   wxTextCtrl *txtFtpUser;
   wxTextCtrl *txtFtpPass;
   wxFTP *ftp;
   wxPoint mousePos;
   wxProgressDialog *progress;

   wxArrayString *filenameList;
   wxArrayString *filepermList;
   wxArrayString *filesizeList;
   wxArrayString *dirnameList;
   wxArrayString *dirpermList;
   wxArrayString *dirsizeList;
   wxArrayString *displayNames;
   wxArrayString *displayPerm;
   wxArrayString *displaySizes;
   wxArrayString *deleteFileList;
   wxArrayString *deleteDirList;
   wxArrayString *ftpList;

   long listIndex;
   long ftpIndex;
   long currentFtp;
   bool abort;

   DECLARE_EVENT_TABLE()
};

#endif

// Indentation settings for Vim and Emacs and unique identifier for Arch, a
// version control system. Please do not modify past this point.
//
// Local Variables:
// c-basic-offset: 3
// indent-tabs-mode: nil
// End:
//
// vim: et sts=3 sw=3



