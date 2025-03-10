#ifndef PATTERN_H
#define PATTERN_H

// Define movement patterns for the laser moving head
enum class Pattern {
    WAVEFORM_SWEEP,      //0 Smooth sinusoidal motion
    BASS_SPIKES,         //1 Random jumps when bass hits
    SPIRAL_EXPANSION,    //2 Expanding spiral pattern
    BPM_SCANNING,        //3 Speed adapts to BPM
    STARBURST,           //4 Radial burst effect
    FIGURE_8_MOTION,     //5 Moves in an infinity loop
    RANDOM_SWIRL //6 Moves in erratic circular patterns with varying speed
};

#endif