/**********************************************************************

  Audacity: A Digital Audio Editor

  MixerBoard.h

  Vaughan Johnson, January 2007

**********************************************************************/

#ifndef __AUDACITY_MIXER_BOARD__
#define __AUDACITY_MIXER_BOARD__

#include <wx/frame.h>
#include <wx/hashmap.h>
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/slider.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>

#include "widgets/AButton.h"
#include "widgets/ASlider.h"
#include "widgets/Meter.h"

class AudacityProject;
class MixerBoard;
class WaveTrack;

class MixerTrackCluster : public wxPanel { 
public:
   MixerTrackCluster(wxWindow* parent, 
                     MixerBoard* grandParent, AudacityProject* project, 
                     WaveTrack* pLeftTrack, WaveTrack* pRightTrack = NULL, 
                     const wxPoint& pos = wxDefaultPosition, 
                     const wxSize& size = wxDefaultSize);
   ~MixerTrackCluster() {};

   void ResetMeter();

   void UpdateHeight(); // For wxSizeEvents, update gain slider and meter.

   // These are used by TrackPanel for synchronizing control states, etc.
   void UpdateName();
   void UpdateMute();
   void UpdateSolo();
   void UpdatePan();
   void UpdateGain();
   void UpdateMeter(double t);

private:
   int GetGainToSliderValue();
   wxColour GetTrackColor();

   // event handlers
   void OnKeyEvent(wxKeyEvent& event);
   void OnMouseEvent(wxMouseEvent& event);
   void OnPaint(wxPaintEvent& evt);

   void OnButton_Mute(wxCommandEvent& event);
   void OnButton_Solo(wxCommandEvent& event);
   void OnSlider_Pan(wxCommandEvent& event);
   void OnSlider_Gain(wxCommandEvent& event);
   void OnSliderScroll_Gain(wxScrollEvent& event);

public:
   WaveTrack* mLeftTrack;
   WaveTrack* mRightTrack; // NULL if mono

private:
   MixerBoard* mMixerBoard;
   AudacityProject* mProject;

   // controls
   wxStaticText* mStaticText_TrackName;
   wxStaticBitmap* mStaticBitmap_MusicalInstrument;
   AButton* mToggleButton_Mute;
   AButton* mToggleButton_Solo;
   ASlider* mSlider_Pan;
   wxSlider* mSlider_Gain; //v ASlider* mSlider_Gain;
   Meter* mMeter;

public:
   DECLARE_EVENT_TABLE()
};

WX_DEFINE_ARRAY(MixerTrackCluster*, MixerTrackClusterArray);


class MusicalInstrument {
public:
   MusicalInstrument(wxBitmap* pBitmap, const wxString strXPMfilename);
   ~MusicalInstrument();

   wxBitmap*      mBitmap;
   wxArrayString  mKeywords;
};
WX_DECLARE_OBJARRAY(MusicalInstrument, MusicalInstrumentArray);



// wxScrolledWindow ignores mouse clicks in client area, 
// but they don't get passed to Mixerboard.
// We need to catch them to deselect all track clusters.
class MixerBoardScrolledWindow : public wxScrolledWindow {
public: 
   MixerBoardScrolledWindow(AudacityProject* project, 
                           MixerBoard* parent, wxWindowID id = -1, 
                           const wxPoint& pos = wxDefaultPosition, 
                           const wxSize& size = wxDefaultSize, 
                           long style = wxHSCROLL | wxVSCROLL);

private:
   void OnMouseEvent(wxMouseEvent& event);

private: 
   MixerBoard* mMixerBoard;
   AudacityProject* mProject;

public:
   DECLARE_EVENT_TABLE()
};


class TrackList;

// MixerBoard is a window that can either be in MixerBoardFrame or AudacityProject frame.
#if (AUDACITY_BRANDING != BRAND_UMIXIT)
   class MixerBoardFrame;
#endif

class MixerBoard : public wxWindow { 
#if (AUDACITY_BRANDING != BRAND_UMIXIT)
   friend class MixerBoardFrame;
#endif

public:
   MixerBoard(AudacityProject* pProject, 
               wxFrame* parent, 
               const wxPoint& pos = wxDefaultPosition, 
               const wxSize& size = wxDefaultSize);
   ~MixerBoard();

   // Add clusters for any tracks we're not yet showing.
   // Update pointers for tracks we're aleady showing. 
   void AddOrUpdateTrackClusters(); 

   int GetTrackClustersWidth();
   void RemoveTrackCluster(const WaveTrack* pLeftTrack);
   void MoveTrackCluster(const WaveTrack* pLeftTrack, bool bUp); // Up in TrackPanel is left in MixerBoard.

   wxBitmap* GetMusicalInstrumentBitmap(const WaveTrack* pLeftTrack);

   bool HasSolo();
   void IncrementSoloCount(int nIncrement = 1);

   void ResetMeters();

   void UniquelyMuteOrSolo(const WaveTrack* pTargetLeftTrack, bool bSolo);

   void UpdateName(const WaveTrack* pLeftTrack);
   void UpdateMute(const WaveTrack* pLeftTrack = NULL); // NULL means update for all tracks.
   void UpdateSolo(const WaveTrack* pLeftTrack = NULL); // NULL means update for all tracks.
   void UpdatePan(const WaveTrack* pLeftTrack);
   void UpdateGain(const WaveTrack* pLeftTrack);
   
   void UpdateMeters(double t);

   void UpdateWidth();

private:
   void CreateMuteSoloImages();
   int FindMixerTrackCluster(const WaveTrack* pLeftTrack, 
                              MixerTrackCluster** hMixerTrackCluster) const;
   void LoadMusicalInstruments();

   // event handlers
   void OnSize(wxSizeEvent &evt);


public:
   // mute & solo button images: Create once and store on MixerBoard for use in all MixerTrackClusters.
   wxImage* mImageMuteUp;
   wxImage* mImageMuteOver;
   wxImage* mImageMuteDown;
   wxImage* mImageMuteDownWhileSolo; // the one actually alternate image
   wxImage* mImageMuteDisabled;
   wxImage* mImageSoloUp;
   wxImage* mImageSoloOver;
   wxImage* mImageSoloDown;
   wxImage* mImageSoloDisabled;

   int mMuteSoloWidth;

private:
   // Track clusters are maintained in the same order as the WaveTracks.
   MixerTrackClusterArray     mMixerTrackClusters; 

   MusicalInstrumentArray     mMusicalInstruments; 
   AudacityProject*           mProject;
   MixerBoardScrolledWindow*  mScrolledWindow; // Holds the MixerTrackClusters and handles scrolling.
   unsigned int               mSoloCount;
   double                     mT;
   TrackList*                 mTracks;

public:
   DECLARE_EVENT_TABLE()
};

#if (AUDACITY_BRANDING != BRAND_UMIXIT)
   class MixerBoardFrame : public wxFrame { 
   public:
      MixerBoardFrame(AudacityProject* parent);
      ~MixerBoardFrame();
   private:
      // event handlers
      void OnCloseWindow(wxCloseEvent &WXUNUSED(event));
      void OnMaximize(wxMaximizeEvent &event);
      void OnSize(wxSizeEvent &evt);

   public:
      MixerBoard* mMixerBoard;

   public:
      DECLARE_EVENT_TABLE()
   };
#endif

#endif // __AUDACITY_MIXER_BOARD__
