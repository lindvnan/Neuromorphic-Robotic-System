# -*- coding: utf-8 -*-

import pyNN.spiNNaker as p
from spynnaker8.utilities import DataHolder
from pacman.model.graphs.application import ApplicationSpiNNakerLinkVertex
from spinn_front_end_common.abstract_models import \
    AbstractProvidesOutgoingPartitionConstraints
from pacman.model.constraints.key_allocator_constraints \
    import FixedKeyAndMaskConstraint
from pacman.model.routing_info import BaseKeyAndMask
from pyNN.utility.plotting import Figure, Panel
import matplotlib.pyplot as plt


class MCU(ApplicationSpiNNakerLinkVertex, AbstractProvidesOutgoingPartitionConstraints,):

    def __init__(self, n_neurons, spinnaker_link_id, board_address=None, label=None):
        super(MCU, self).__init__(
            n_atoms=n_neurons, spinnaker_link_id=spinnaker_link_id,
            board_address=board_address, label=label)
        
    def get_outgoing_partition_constraints(self, partition):
        return [FixedKeyAndMaskConstraint(
            [BaseKeyAndMask(0x12340000, 0xFFFFFFF0)])]
    
class  MCUDataHolder(DataHolder):
    def __init__(
            self, spinnaker_link_id,
            board_address=None,
            label=None):

        DataHolder.__init__(
            self, {
                'spinnaker_link_id': spinnaker_link_id,
                'board_address': board_address, 'label': label})
    
    @staticmethod
    def build_model():
        return  MCU

class MyMotor(ApplicationSpiNNakerLinkVertex):

    def __init__(self, n_neurons, spinnaker_link_id, board_address=None, label=None):

        super(MyMotor, self).__init__(

            n_atoms=n_neurons, spinnaker_link_id=spinnaker_link_id,

            board_address=board_address, label=label)

class MyMotorDataHolder(DataHolder):
    def __init__(
            self, spinnaker_link_id,
            board_address=None,
            label=None):

        DataHolder.__init__(
            self, {
                'spinnaker_link_id': spinnaker_link_id,
                'board_address': board_address, 'label': label})
    
    @staticmethod
    def build_model():
        return MyMotor
    
    
# set up the tools
tstop = 60000
# tstop = 1000
p.setup(timestep=1.0, min_delay=1.0, max_delay=32.0)


input_population = p.Population(8,MCUDataHolder(spinnaker_link_id=0))
control_population = p.Population(8, p.IF_curr_exp())

motor_device = p.Population(8, MyMotorDataHolder(spinnaker_link_id=0))

p.Projection(
    input_population, control_population, p.OneToOneConnector(),
    synapse_type=p.StaticSynapse(weight=5, delay=3))

p.external_devices.activate_live_output_to(control_population, motor_device)

# === Setup recording ===
control_population.record("spikes")

p.run(tstop)

# === Plot results ===
control_spikes = control_population.get_data("spikes")

Figure(
    # raster plot of the presynaptic neuron spike times
    Panel(control_spikes.segments[0].spiketrains, xlabel="Time/ms", xticks=True,
          yticks=True, markersize=3, xlim=(0, tstop)),
    title="CONTROL: spikes",
    annotations="Simulated with {}".format(p.name())
)
    
#plt.savefig('IO3.png', dpi = 400)
plt.show()

p.end()


