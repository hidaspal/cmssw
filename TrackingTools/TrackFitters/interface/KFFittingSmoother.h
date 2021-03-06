#ifndef CD_KFFittingSmoother_H_
#define CD_KFFittingSmoother_H_

/** \class KFFittingSmoother
 *  A TrajectorySmoother that rpeats the forward fit before smoothing.
 *  This is necessary e.g. when the seed introduced a bias (by using
 *  a beam contraint etc.). Ported from ORCA
 *
 *  $Date: 2012/09/13 08:48:07 $
 *  $Revision: 1.16.2.1 $
 *  \author todorov, cerati
 */

#include "TrackingTools/PatternTools/interface/TrajectorySmoother.h"
#include "TrackingTools/TrackFitters/interface/TrajectoryFitter.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "TrackingTools/TrackFitters/interface/TrajectoryStateWithArbitraryError.h"

class KFFittingSmoother : public TrajectoryFitter {

public:
  /// constructor with predefined fitter and smoother and propagator
  KFFittingSmoother(const TrajectoryFitter& aFitter,
                    const TrajectorySmoother& aSmoother,
		    double estimateCut = -1,
		    double logPixelProbabilityCut = -16.0, 
		    int minNumberOfHits = 5,
		    bool rejectTracks = false,
		    bool BreakTrajWith2ConsecutiveMissing = false,
		    bool NoInvalidHitsBeginEnd = false) :
    theFitter(aFitter.clone()),
    theSmoother(aSmoother.clone()),
    theEstimateCut(estimateCut),

    // ggiurgiu@fnal.gov
    theLogPixelProbabilityCut( logPixelProbabilityCut ),
    
    theMinNumberOfHits(minNumberOfHits),
    rejectTracksFlag(rejectTracks),
    breakTrajWith2ConsecutiveMissing(BreakTrajWith2ConsecutiveMissing),
    noInvalidHitsBeginEnd(NoInvalidHitsBeginEnd) {}
  
  virtual ~KFFittingSmoother();
  
  virtual std::vector<Trajectory> fit(const Trajectory& t) const;
  virtual std::vector<Trajectory> fit(const TrajectorySeed& aSeed,
				 const RecHitContainer& hits, 
				 const TrajectoryStateOnSurface& firstPredTsos) const;
  virtual std::vector<Trajectory> fit(const TrajectorySeed& aSeed,
				 const RecHitContainer& hits) const;

  const TrajectoryFitter* fitter() const {return theFitter;}
  const TrajectorySmoother* smoother() const {return theSmoother;}

  KFFittingSmoother* clone() const {
    return new KFFittingSmoother(*theFitter,*theSmoother,
				 theEstimateCut,theLogPixelProbabilityCut,
				 theMinNumberOfHits,rejectTracksFlag,
				 breakTrajWith2ConsecutiveMissing,noInvalidHitsBeginEnd);
  }
  
private:

  const TrajectoryFitter* theFitter;
  const TrajectorySmoother* theSmoother;
  double theEstimateCut;

  double theLogPixelProbabilityCut; // ggiurgiu@fnal.gov

  int theMinNumberOfHits;
  bool rejectTracksFlag;
  bool breakTrajWith2ConsecutiveMissing;
  bool noInvalidHitsBeginEnd;
  
  void smoothingStep(std::vector<Trajectory>& fitted, std::vector<Trajectory> & smoothed) const;
  TrajectoryStateWithArbitraryError   tsosWithError;

  /// Method to check that the trajectory has no NaN in the states and chi2
  bool checkForNans(const Trajectory &theTraj) const;
  
};

#endif //CD_KFFittingSmoother_H_
