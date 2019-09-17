#ifndef MOCO_MOCOINVERSE_H
#define MOCO_MOCOINVERSE_H
/* -------------------------------------------------------------------------- *
 * OpenSim Moco: MocoInverse.h                                                *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2019 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Christopher Dembia                                              *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0          *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "Common/TableProcessor.h"
#include "MocoTool.h"
#include "MocoTrajectory.h"
#include "MocoStudy.h"
#include "osimMocoDLL.h"

#include <OpenSim/Simulation/Model/Model.h>

namespace OpenSim {

class MocoInverse;

/// This class holds the solution from MocoInverse.
class MocoInverseSolution {
public:
    const MocoSolution& getMocoSolution() const { return m_mocoSolution; }
    const TimeSeriesTable& getOutputs() const { return m_outputs; }
private:
    void setMocoSolution(MocoSolution mocoSolution) {
        m_mocoSolution = std::move(mocoSolution);
    }
    void setOutputs(TimeSeriesTable outputs) {
        m_outputs = std::move(outputs);
    }
    MocoSolution m_mocoSolution;
    TimeSeriesTable m_outputs;
    friend class MocoInverse;
};

/// This tool solves problems in which the kinematics are prescribed and you
/// seek the actuator (e.g., muscle) behavior that may have given rise to the
/// provided kinematics. The term "inverse" describes methods that estimate
/// quantities from an observation; on the other hand, "forward" methods attempt
/// to predict (unobserved) behavior. In this case, "inverse" refers to the
/// multibody systems. This class can still be used to simulate muscles in a
/// "forward" or predictive sense.
///
/// The kinematics file must provide values for all coordinates (even those
/// labeled as dependent in a CoordinateCouplerConstraint); missing coordinates
/// are set to NaN.
///
/// The provided trajectory is altered to satisfy any enabled kinematic
/// constraints in the model.
///
/// Cost
/// ----
/// By default, MocoInverse minimizes the sum of squared controls and
/// constrains initial activation to be equal to initial excitation (to avoid
/// initial activation spikes). To customize the cost, invoke initialize(), add
/// costs manually, and solve the problem using the solver directly. Note,
/// however, that kinematic states are not included in the solution if you use
/// the solver directly.
///
/// Default solver settings
/// -----------------------
/// - solver: MocoCasADiSolver
/// - dynamics_mode: implicit
/// - transcription_sceheme: trapezoidal
/// - optim_convergence_tolerance: 1e-3
/// - optim_constraint_tolerance: 1e-3
/// - optim_sparsity_detection: random
/// - optim_finite_difference_scheme: forward
///
/// ### Cost
///
/// MocoInverse minimizes the sum of squared controls and, optionally, the sum
/// of squared states. MocoInverse assumes that the only states in the system
/// are muscle activations, but this is not checked or enforced.
/// Currently, the costs used by MocoInverse cannot be customized.
/// As MocoInverse becomes more mature and general, the costs will become more
/// flexible.
///
/// ### Mesh interval
///
/// A smaller mesh interval increases the convergence time, but is necessary
/// for fast motions or problems with stiff differential equations (e.g.,
/// stiff tendons).
/// For gait, consider using a mesh interval between 0.01 and 0.05 seconds.
/// Try solving your problem with decreasing mesh intervals and choose a mesh
/// interval at which the solution stops changing noticeably.
///
/// @underdevelopment
class OSIMMOCO_API MocoInverse : public MocoTool {
    OpenSim_DECLARE_CONCRETE_OBJECT(MocoInverse, MocoTool);

public:

    OpenSim_DECLARE_PROPERTY(kinematics, TableProcessor,
            "Generalized coordinate values to prescribe.");

    OpenSim_DECLARE_PROPERTY(kinematics_allow_extra_columns, bool,
            "Allow the kinematics file to contain columns that do not name "
            "states in the model. "
            "This is false by default to help you avoid accidents.");

    OpenSim_DECLARE_PROPERTY(minimize_sum_squared_states, bool,
            "Minimize the sum of squared states (e.g., activations). "
            "Do not use this if tendon compliance is enabled. Default: false.");

    OpenSim_DECLARE_OPTIONAL_PROPERTY(max_iterations, int,
            "Maximum number of solver iterations (default: solver default).");

    OpenSim_DECLARE_PROPERTY(tolerance, double,
            "The convergence and constraint tolerances (default: 1e-3).");

    OpenSim_DECLARE_LIST_PROPERTY(output_paths, std::string,
            "Outputs to compute after solving the problem."
            " Entries can be regular expressions (e.g., '.*activation').");

    MocoInverse() { constructProperties(); }

    void setKinematics(TableProcessor kinematics) {
        set_kinematics(std::move(kinematics));
    }

    MocoStudy initialize() const;
    /// Solve the problem returned by initialize() and compute the outputs
    /// listed in output_paths.
    MocoInverseSolution solve() const;

private:
    void constructProperties();
    std::pair<MocoStudy, TimeSeriesTable> initializeInternal() const;
};

} // namespace OpenSim

#endif // MOCO_MOCOINVERSE_H