# REF_resources_links.md -- Curated Resource List for Real-Time Audio Analysis

**Document**: REF_resources_links.md
**Scope**: Annotated bibliography of papers, books, libraries, communities, datasets, tools, and standards relevant to building a real-time audio analysis engine for music visualization.

> **Cross-references**: Every document in this research library. This is the master reference index.
> [ARCH_pipeline.md](ARCH_pipeline.md) | [ARCH_audio_io.md](ARCH_audio_io.md) | [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md) | [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md) | [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md) | [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md) | [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) | [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) | [FEATURES_transients_texture.md](FEATURES_transients_texture.md) | [FEATURES_structural.md](FEATURES_structural.md) | [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) | [LIB_essentia.md](LIB_essentia.md) | [LIB_aubio.md](LIB_aubio.md) | [LIB_juce.md](LIB_juce.md) | [LIB_fft_comparison.md](LIB_fft_comparison.md) | [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md) | [REF_latency_numbers.md](REF_latency_numbers.md) | [REF_math_reference.md](REF_math_reference.md) | [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) | [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) | [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md) | [IMPL_testing_validation.md](IMPL_testing_validation.md) | [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md)

---

## 1. Key Academic Papers

Organized by topic. Each entry includes the full citation, a DOI or URL where available, and a brief annotation explaining relevance to this project.

### 1.1 Onset Detection

**Bello, J.P., Daudet, L., Abdallah, S., Duxbury, C., Davies, M., & Sandler, M.B. (2005). "A Tutorial on Onset Detection in Music Signals." IEEE Transactions on Speech and Audio Processing, 13(5), 1035-1047.**
DOI: `10.1109/TSA.2005.851998`
The definitive survey of onset detection methods. Covers spectral flux, high-frequency content (HFC), complex-domain ODF, phase deviation, and adaptive thresholding. Required reading before implementing any onset detector. Directly referenced in [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) and [FEATURES_transients_texture.md](FEATURES_transients_texture.md).

**Böck, S., Krebs, F., & Widmer, G. (2012). "Evaluating the Online Capabilities of Onset Detection Methods." Proceedings of the 13th International Society for Music Information Retrieval Conference (ISMIR).**
URL: https://ismir2012.ismir.net/
Evaluates onset detection in a causal (online) setting rather than the typical offline evaluation paradigm. Essential for understanding which onset methods work in real-time with bounded lookahead. Directly relevant to the latency constraints discussed in [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md).

**Böck, S., & Widmer, G. (2013). "Maximum Filter Vibrato Suppression for Onset Detection." Proceedings of the 16th International Conference on Digital Audio Effects (DAFx).**
URL: https://dafx13.nuim.ie/
Addresses a practical problem: vibrato and tremolo cause false onset detections. The maximum-filter approach suppresses spectral flux modulations from vibrato while preserving genuine onsets. Useful for live vocal or string input.

### 1.2 Beat Tracking and Tempo Estimation

**Dixon, S. (2007). "Evaluation of the Audio Beat Tracking System BeatRoot." Computer Music Journal, 31(1), 39-51.**
DOI: `10.1162/comj.2007.31.1.39`
Describes the BeatRoot system: a multi-agent approach where competing beat hypotheses are maintained and scored. Important as a reference system and for its evaluation methodology. Referenced in [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md).

**Ellis, D.P.W. (2007). "Beat Tracking by Dynamic Programming." Journal of New Music Research, 36(1), 51-60.**
DOI: `10.1080/09298210701653344`
URL: https://www.ee.columbia.edu/~dpwe/pubs/Ellis07-beattrack.pdf
Frames beat tracking as a global optimization problem solved by dynamic programming. The algorithm constructs an onset strength envelope, computes autocorrelation for tempo estimation, then uses dynamic programming to find the beat sequence maximizing both onset alignment and tempo consistency. Simpler and more robust than multi-agent methods for many genres. Widely cited and implemented in librosa.

**Scheirer, E.D. (1998). "Tempo and Beat Analysis of Acoustic Musical Signals." Journal of the Acoustical Society of America, 103(1), 588-601.**
DOI: `10.1121/1.421129`
Foundational work on computational tempo estimation. Introduces the filterbank-comb-filter-bank architecture: audio is split into frequency bands, each band's envelope is extracted, and a bank of comb filters (resonators at candidate tempi) selects the dominant periodicity. This architecture remains the basis of many real-time tempo estimators. Referenced in [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md).

**Böck, S., Krebs, F., & Widmer, G. (2015). "Accurate Tempo Estimation Based on Recurrent Neural Networks and Resonating Comb Filters." Proceedings of the 16th ISMIR Conference.**
URL: https://ismir2015.ismir.net/
Combines deep learning (bidirectional LSTM for beat activation) with traditional comb filtering for tempo. Achieves state-of-the-art accuracy. The neural network component is not suitable for real-time on CPU, but the comb filter post-processing is applicable to any onset strength envelope.

**Davies, M.E.P., & Plumbley, M.D. (2007). "Context-Dependent Beat Tracking of Musical Audio." IEEE Transactions on Audio, Speech, and Language Processing, 15(3), 1009-1020.**
DOI: `10.1109/TASL.2006.885257`
Introduces context-dependent beat tracking that adapts to changes in tempo and rhythmic complexity. Uses a two-state model (tracking vs. recovery) that is well-suited to live performance where tempo may shift.

**Zapata, J.R., Davies, M.E.P., & Gómez, E. (2014). "Multi-Feature Beat Tracking." IEEE/ACM Transactions on Audio, Speech, and Language Processing, 22(4), 816-825.**
DOI: `10.1109/TASLP.2014.2305252`
Combines multiple onset detection functions (spectral flux, complex domain, etc.) to produce a more robust beat tracker. Directly relevant to the multi-feature approach recommended in [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md).

### 1.3 Pitch Detection

**de Cheveigné, A., & Kawahara, H. (2002). "YIN, a Fundamental Frequency Estimator for Speech and Music." Journal of the Acoustical Society of America, 111(4), 1917-1930.**
DOI: `10.1121/1.1458024`
The original YIN paper. Introduces the cumulative mean normalized difference function (CMND) and parabolic interpolation for sub-sample period estimation. YIN remains the most widely used monophonic pitch detector in real-time systems due to its simplicity, accuracy, and O(N*tau_max) complexity. Core algorithm in [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md). Implemented in aubio and Essentia.

**Mauch, M., & Dixon, S. (2014). "pYIN: A Fundamental Frequency Estimator Using Probabilistic YIN." Proceedings of the IEEE International Conference on Acoustics, Speech and Signal Processing (ICASSP), 659-663.**
DOI: `10.1109/ICASSP.2014.6853678`
Extends YIN with a hidden Markov model (HMM) that tracks pitch trajectories over time, reducing octave errors and improving robustness in noisy or polyphonic conditions. The HMM adds latency (needs a few frames of context) but is suitable for near-real-time use. Implemented in the Vamp plugin ecosystem and Sonic Visualiser.

**Salamon, J., & Gómez, E. (2012). "Melody Extraction from Polyphonic Music Signals Using Pitch Salience Functions." IEEE Transactions on Audio, Speech, and Language Processing, 20(6), 1688-1701.**
DOI: `10.1109/TASL.2012.2188515`
The MELODIA algorithm for predominant melody extraction from polyphonic audio. Uses pitch salience functions computed from the spectral peaks. More expensive than YIN but handles polyphonic content. Available in Essentia as `PredominantPitchMelodia`. Referenced in [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md).

### 1.4 Spectral Analysis and Transforms

**Brown, J.C. (1991). "Calculation of a Constant Q Spectral Transform." Journal of the Acoustical Society of America, 89(1), 425-434.**
DOI: `10.1121/1.400476`
Introduces the Constant-Q Transform (CQT), where frequency bins are logarithmically spaced and each bin's bandwidth is a fixed fraction of its center frequency. This matches musical pitch spacing (12 bins per octave = semitone resolution) and provides better frequency resolution at low frequencies than the FFT. Foundation for chroma features and harmonic analysis in [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md) and [FEATURES_spectral.md](FEATURES_spectral.md).

**Schörkhuber, C., & Klapuri, A. (2010). "Constant-Q Transform Toolbox for Music Processing." Proceedings of the 7th Sound and Music Computing Conference (SMC).**
URL: https://smc2010.smcnetwork.org/
Presents an efficient implementation of the CQT using kernel-based sparse multiplication, making it practical for real-time use. The approach reduces the CQT from O(N * K * Q) to near-O(N log N) by leveraging the FFT.

**McFee, B., & Bello, J.P. (2020). "Open-source practices for music signal processing research: Recommendations for transparent, sustainable, and reproducible audio research." IEEE Signal Processing Magazine, 36(1), 128-137.**
Advocates for reproducibility in MIR research. The authors maintain librosa, which implements many of the algorithms referenced throughout this library.

### 1.5 Source Separation (HPSS)

**Fitzgerald, D. (2010). "Harmonic/Percussive Separation Using Median Filtering." Proceedings of the 13th International Conference on Digital Audio Effects (DAFx-10).**
URL: https://arrow.tudublin.ie/argcon/67/
Introduces median-filtering-based harmonic/percussive source separation (HPSS). A spectrogram is filtered horizontally (time axis) to extract harmonic content and vertically (frequency axis) to extract percussive content. Computationally cheap and effective. Core algorithm in [FEATURES_transients_texture.md](FEATURES_transients_texture.md), used to split audio into harmonic and percussive streams for independent feature extraction and visualization.

**Driedger, J., Müller, M., & Disch, S. (2014). "Extending Harmonic-Percussive Separation of Audio Signals." Proceedings of the 15th ISMIR Conference.**
Extends HPSS with a residual component (harmonic + percussive + residual). The residual captures content that is neither clearly harmonic nor percussive (e.g., breathy vocals, sustained noise). Potentially useful for a third visualization channel.

### 1.6 Psychoacoustics and Perception

**Zwicker, E., & Fastl, H. (2007). Psychoacoustics: Facts and Models. 3rd Edition. Springer.**
ISBN: `978-3-540-23159-2`
The comprehensive reference for psychoacoustic models: loudness (Zwicker model), critical bands (Bark scale), roughness, fluctuation strength, sharpness, and masking. These models underpin perceptually motivated feature extraction discussed in [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) and [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md). The fluctuation strength model is directly used for texture analysis in [FEATURES_transients_texture.md](FEATURES_transients_texture.md).

**Krumhansl, C.L. (1990). Cognitive Foundations of Musical Pitch. Oxford University Press.**
ISBN: `978-0-19-505475-0`
Defines the key-finding profiles (Krumhansl-Schmuckler profiles): empirically derived weights for each pitch class that characterize major and minor keys. These profiles are correlated with chroma vectors to estimate musical key. Referenced in [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md).

**Plomp, R., & Levelt, W.J.M. (1965). "Tonal Consonance and Critical Bandwidth." Journal of the Acoustical Society of America, 38(4), 548-560.**
DOI: `10.1121/1.1909741`
Establishes the relationship between sensory consonance/dissonance and critical bandwidth. Two tones within approximately one-quarter of a critical band apart produce maximal roughness (dissonance). This model underlies the roughness and dissonance features in [FEATURES_spectral.md](FEATURES_spectral.md).

**Moore, B.C.J. (2012). An Introduction to the Psychology of Hearing. 6th Edition. Brill.**
ISBN: `978-1-78050-038-1`
Comprehensive psychoacoustics textbook. More accessible than Zwicker & Fastl for readers without a strong background in auditory physiology. Covers loudness perception, pitch perception, temporal resolution, and masking.

**Patterson, R.D., Robinson, K., Holdsworth, J., McKeown, D., Zhang, C., & Allerhand, M. (1992). "Complex Sounds and Auditory Images." Auditory Physiology and Perception, 429-446.**
Describes the Gammatone filterbank, an auditory-inspired filterbank that models the frequency selectivity of the basilar membrane. Widely used in computational auditory scene analysis (CASA). An alternative to Mel filterbanks for perceptually motivated frequency decomposition (see [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md)).

### 1.7 MFCCs, Mel Scale, and Timbre

**Davis, S.B., & Mermelstein, P. (1980). "Comparison of Parametric Representations for Monosyllabic Word Recognition in Continuously Spoken Sentences." IEEE Transactions on Acoustics, Speech, and Signal Processing, 28(4), 357-366.**
DOI: `10.1109/TASSP.1980.1163420`
The original MFCC paper. Introduces Mel-frequency cepstral coefficients for speech recognition, but the representation is equally fundamental for music timbre description. The first 13 MFCCs capture the spectral envelope shape, which is the primary cue for timbre. Core feature in [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md).

**Stevens, S.S., Volkmann, J., & Newman, E.B. (1937). "A Scale for the Measurement of the Psychological Magnitude of Pitch." Journal of the Acoustical Society of America, 8(3), 185-190.**
DOI: `10.1121/1.1915893`
Defines the Mel scale: a perceptual pitch scale where equal Mel intervals correspond to perceptually equal pitch differences. Foundation for Mel filterbanks used throughout this project. Referenced in [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md) and [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md).

### 1.8 Structural Analysis and Self-Similarity

**Foote, J. (2000). "Automatic Audio Segmentation Using a Measure of Audio Novelty." Proceedings of the IEEE International Conference on Multimedia and Expo (ICME), 452-455.**
DOI: `10.1109/ICME.2000.869637`
Introduces the novelty function derived from a self-similarity matrix. Convolves a checkerboard kernel along the diagonal of the self-similarity matrix to detect structural boundaries (verse-chorus, intro-verse, etc.). Core technique in [FEATURES_structural.md](FEATURES_structural.md).

**Müller, M., & Ewert, S. (2011). "Chroma Toolbox: MATLAB Implementations for Extracting Variants of Chroma-Based Audio Features." Proceedings of the 12th ISMIR Conference.**
URL: https://ismir2011.ismir.net/
Presents efficient implementations of CQT-based chroma (CQ chroma), energy-normalized chroma (CENS), and chroma with short-time statistics. These chroma variants feed structural analysis algorithms. Useful reference implementation.

**Serra, J., Müller, M., Grosche, P., & Arcos, J.L. (2012). "Unsupervised Detection of Music Boundaries by Time Series Structure Features." Proceedings of the 26th AAAI Conference.**
Uses lag-embedded time series features for boundary detection. More robust than raw feature novelty for certain music types.

### 1.9 Deep Learning for Audio

**Böck, S., & Schedl, M. (2012). "Polyphonic Piano Note Transcription with Recurrent Neural Networks." Proceedings of the IEEE International Conference on Acoustics, Speech and Signal Processing (ICASSP), 3387-3390.**
DOI: `10.1109/ICASSP.2012.6288141`
Early application of RNNs to music transcription. Demonstrates that neural networks can learn complex spectral-to-note mappings that handcrafted features miss. Historically significant as it helped catalyze the deep learning wave in MIR. Not directly applicable to real-time CPU inference, but the mel-spectrogram input pipeline is relevant.

**Choi, K., Fazekas, G., Sandler, M., & Cho, K. (2017). "Convolutional Recurrent Neural Networks for Music Classification." Proceedings of the IEEE International Conference on Acoustics, Speech and Signal Processing (ICASSP).**
DOI: `10.1109/ICASSP.2017.7952585`
CNN-RNN hybrid for music tagging. Demonstrates that convolutional layers on mel-spectrograms followed by RNNs can capture both local spectral patterns and temporal dynamics. Relevant if pre-trained models are used for genre/mood classification to drive visualization themes.

**Kong, Q., Cao, Y., Iqbal, T., Wang, Y., Wang, W., & Plumbley, M.D. (2020). "PANNs: Large-Scale Pretrained Audio Neural Networks for Audio Pattern Recognition." IEEE/ACM Transactions on Audio, Speech, and Language Processing, 28, 2880-2894.**
DOI: `10.1109/TASLP.2020.3030497`
URL: https://github.com/qiuqiangkong/audioset_tagging_cnn
Pretrained CNN models on AudioSet (2M clips, 527 classes). Can be used for real-time audio event detection and classification. The CNN14 model processes 1 second of audio in ~5ms on GPU, making near-real-time inference feasible for event-driven visualization triggers.

**Défossez, A., Synnaeve, G., & Adi, Y. (2022). "Real Time Speech Enhancement in the Waveform Domain." Proceedings of Interspeech.**
URL: https://github.com/facebookresearch/denoiser
Demonstrates real-time neural audio processing on CPU using causal convolutions and efficient architectures. While focused on speech, the engineering techniques (streaming inference, model pruning, ONNX export) are directly relevant to deploying any neural audio model in real-time.

### 1.10 Loudness and Dynamics

**ITU-R BS.1770-5 (2023). "Algorithms to Measure Audio Programme Loudness and True-Peak Audio Level."**
URL: https://www.itu.int/rec/R-REC-BS.1770
The international standard for loudness measurement. Defines the K-weighting filter and LUFS/LKFS measurement. Essential for loudness normalization of input audio. Referenced in [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md).

**Giannoulis, D., Massberg, M., & Reiss, J.D. (2012). "Digital Dynamic Range Compressor Design -- A Tutorial and Analysis." Journal of the Audio Engineering Society, 60(6), 399-408.**
URL: https://www.aes.org/e-lib/browse.cfm?elib=16354
Comprehensive tutorial on compressor/limiter design: attack/release envelopes, knee curves, gain computation. Understanding compressor behavior is essential for implementing the dynamic range and crest factor features in [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md).

### 1.11 Audio Effects and DSP

**Smith, J.O. (2007). "Introduction to Digital Filters with Audio Applications." CCRMA, Stanford.**
URL: https://ccrma.stanford.edu/~jos/filters/
Free online book. Covers IIR and FIR filter design, biquad sections, and practical considerations for audio. Directly relevant to implementing the K-weighting filter, A-weighting, and any pre-filtering in the analysis pipeline.

**Välimäki, V., & Reiss, J.D. (2016). "All About Audio Equalization: Solutions and Frontiers." Applied Sciences, 6(5), 129.**
DOI: `10.3390/app6050129`
Survey of audio equalization techniques. Relevant for designing the frequency-band splitting filters in [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md).

### 1.12 Music Information Retrieval -- General

**Müller, M. (2015). Fundamentals of Music Processing: Audio, Analysis, Algorithms, Applications. Springer.**
DOI: `10.1007/978-3-319-21945-5`
URL: https://www.audiolabs-erlangen.de/fau/professor/mueller/bookFMP
The definitive MIR textbook. Covers audio representations, music synchronization, chord recognition, tempo analysis, structure analysis, and audio retrieval. Companion Jupyter notebooks provide working Python implementations. Referenced across nearly every FEATURES document. Read this first.

**Klapuri, A., & Davy, M. (Eds.) (2006). Signal Processing Methods for Music Transcription. Springer.**
ISBN: `978-0-387-30667-4`
Multi-author volume covering pitch estimation, rhythm analysis, source separation, and multi-pitch detection. More depth than Müller on specific transcription topics.

**Downie, J.S. (2003). "Music Information Retrieval." Annual Review of Information Science and Technology, 37(1), 295-340.**
DOI: `10.1002/aris.1440370108`
Early survey of MIR as a field. Useful for historical context and understanding the problem space.

### 1.13 Additional Relevant Papers

**Peeters, G. (2004). "A Large Set of Audio Features for Sound Description (Similarity and Classification) in the CUIDADO Project." IRCAM Technical Report.**
URL: http://recherche.ircam.fr/anasyn/peeters/ARTICLES/Peeters_2003_cuidadoaudiofeatures.pdf
Catalogs dozens of audio features (spectral, temporal, harmonic, perceptual) with mathematical definitions. A useful reference when implementing the features across this library's FEATURES documents.

**Lartillot, O., & Toiviainen, P. (2007). "A Matlab Toolbox for Musical Feature Extraction from Audio." Proceedings of the 10th International Conference on Digital Audio Effects (DAFx).**
Describes MIRtoolbox, a comprehensive MATLAB feature extraction library. While not directly usable in C++, its feature definitions serve as reference implementations for validation.

**Bogdanov, D., Wack, N., Gómez, E., Gulati, S., Herrera, P., Mayor, O., Roma, G., Salamon, J., Zapata, J.R., & Serra, X. (2013). "ESSENTIA: an Audio Analysis Library for Music Information Retrieval." Proceedings of the 14th ISMIR Conference.**
URL: https://essentia.upf.edu/
The Essentia reference paper. Describes the architecture (standard vs. streaming modes), algorithm catalog, and design philosophy. Required reading before using Essentia. See [LIB_essentia.md](LIB_essentia.md).

**Brossier, P.M. (2006). "Automatic Annotation of Musical Audio for Interactive Applications." PhD Thesis, Queen Mary University of London.**
URL: https://aubio.org/phd/
The aubio reference thesis. Covers onset detection, pitch detection, and tempo estimation algorithms as implemented in aubio. See [LIB_aubio.md](LIB_aubio.md).

**McFee, B., Raffel, C., Liang, D., Ellis, D.P.W., McVicar, M., Battenberg, E., & Nieto, O. (2015). "librosa: Audio and Music Signal Analysis in Python." Proceedings of the 14th Python in Science Conference (SciPy).**
DOI: `10.25080/Majora-7b98e3ed-003`
URL: https://librosa.org/
The librosa reference paper. librosa is the de facto Python library for audio analysis and is used extensively for prototyping, validation, and reference implementations throughout this project.

**Raffel, C., McFee, B., Humphrey, E.J., Salamon, J., Nieto, O., Liang, D., & Ellis, D.P.W. (2014). "mir_eval: A Transparent Implementation of Common MIR Metrics." Proceedings of the 15th ISMIR Conference.**
URL: https://craffel.github.io/mir_eval/
Standardized evaluation metrics for MIR tasks (onset detection F-measure, beat tracking F-measure, melody accuracy, etc.). Use this for quantitative validation of feature extractors. See [IMPL_testing_validation.md](IMPL_testing_validation.md).

**Shao, X., Xu, C., & Kankanhalli, M.S. (2004). "Unsupervised Classification of Music Genre Using Hidden Markov Model." Proceedings of the IEEE International Conference on Multimedia and Expo.**
Demonstrates genre classification using MFCCs and HMMs. Relevant as a baseline approach for automatic genre detection that could drive visualization theme selection.

**Tzanetakis, G., & Cook, P. (2002). "Musical Genre Classification of Audio Signals." IEEE Transactions on Speech and Audio Processing, 10(5), 293-302.**
DOI: `10.1109/TSA.2002.800560`
Introduces the GTZAN dataset and a feature-based genre classification system using MFCCs, spectral features, and rhythm features. A foundational MIR paper that defined the standard feature set for music classification.

---

## 2. Books

Ordered from most to least immediately relevant to this project.

### 2.1 Core References

**Müller, M. (2015). Fundamentals of Music Processing. Springer.**
URL: https://www.audiolabs-erlangen.de/fau/professor/mueller/bookFMP
What it covers: Audio representations (waveform, STFT, CQT, chroma), onset/beat/tempo analysis, chord recognition, structure analysis, audio retrieval, music synchronization. Companion Jupyter notebooks in Python.
When to read: First. This is the single most relevant book. Covers nearly every feature in the FEATURES documents at both mathematical and implementation levels.

**Zölzer, U. (Ed.) (2011). DAFX: Digital Audio Effects. 2nd Edition. Wiley.**
ISBN: `978-0-470-66599-2`
What it covers: Filters, delays, modulation effects, spatial effects, time-stretching, pitch-shifting, spectral processing, and adaptive effects. Each chapter includes block diagrams and pseudocode.
When to read: When implementing any signal processing stage in the pipeline. Particularly relevant for filter design (K-weighting, band-splitting), envelope followers, and spectral processing. Referenced in [ARCH_pipeline.md](ARCH_pipeline.md).

**Oppenheim, A.V., & Schafer, R.W. (2009). Discrete-Time Signal Processing. 3rd Edition. Pearson.**
ISBN: `978-0-13-198842-2`
What it covers: Sampling theory, Z-transforms, DFT/FFT, filter design (FIR and IIR), multirate signal processing, cepstral analysis. The "DSP bible."
When to read: When you need deep mathematical understanding of any DSP concept. Not a practical cookbook -- read Zölzer or Smith for implementations.

**Smith, J.O. (2007). Mathematics of the Discrete Fourier Transform (DFT) with Audio Applications. 2nd Edition.**
URL: https://ccrma.stanford.edu/~jos/mdft/ (free online)
What it covers: Complex numbers, sinusoids, the DFT, FFT, windowing, spectral leakage, zero-padding, interpolation. Written for audio practitioners.
When to read: When any FFT-related concept is unclear. The explanations of windowing and spectral leakage are particularly good. Referenced in [FEATURES_spectral.md](FEATURES_spectral.md).

### 2.2 Perceptual and Cognitive

**Bregman, A.S. (1990). Auditory Scene Analysis: The Perceptual Organization of Sound. MIT Press.**
ISBN: `978-0-262-52195-6`
What it covers: How the auditory system groups frequency components into perceived objects (streams). Sequential grouping, simultaneous grouping, schema-based segregation.
When to read: When designing visualization mappings that should feel "natural." Understanding stream segregation helps decide which audio features should drive independent visual elements. Referenced in [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md).

**Zwicker, E., & Fastl, H. (2007). Psychoacoustics: Facts and Models. 3rd Edition. Springer.**
ISBN: `978-3-540-23159-2`
What it covers: Loudness, pitch perception, critical bands, masking, roughness, fluctuation strength, sharpness. Quantitative models with equations.
When to read: When implementing perceptual features. The loudness, roughness, and fluctuation strength models are directly used in [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) and [FEATURES_transients_texture.md](FEATURES_transients_texture.md).

**Moore, B.C.J. (2012). An Introduction to the Psychology of Hearing. 6th Edition. Brill.**
ISBN: `978-1-78050-038-1`
What it covers: Same domain as Zwicker & Fastl but more accessible. Covers loudness, pitch, temporal processing, spatial hearing, and auditory attention.
When to read: As a gentler introduction before tackling Zwicker & Fastl.

### 2.3 Implementation-Focused

**Pirkle, W. (2019). Designing Audio Effect Plugins in C++: For AAX, AU, and VST3 with DSP Theory. 2nd Edition. Routledge.**
ISBN: `978-1-138-59107-3`
What it covers: Practical C++ DSP: oscillators, filters, delays, dynamics processors, reverb, modulation effects. Uses a cross-platform plugin framework. Includes biquad filter cookbook implementations.
When to read: When writing C++ DSP code. The biquad filter implementations are directly useful for the K-weighting and band-splitting filters. The envelope follower chapter is relevant to [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md).

**Roads, C. (1996). The Computer Music Tutorial. MIT Press.**
ISBN: `978-0-262-68082-0`
What it covers: Everything -- synthesis, sampling, signal processing, spatial audio, psychoacoustics, music representation, composition. 1,200+ pages.
When to read: As a reference encyclopedia. Not cover-to-cover reading, but invaluable when you need background on any computer music topic.

**Steiglitz, K. (1996). A Digital Signal Processing Primer: With Applications to Digital Audio and Computer Music. Addison-Wesley.**
ISBN: `978-0-8053-1684-3`
What it covers: Gentle introduction to DSP with audio focus. Sampling, aliasing, filters, FFT, spectral analysis. Good for building intuition.
When to read: If Oppenheim & Schafer feels too dense.

**Smith, J.O. (2010). Physical Audio Signal Processing. CCRMA, Stanford.**
URL: https://ccrma.stanford.edu/~jos/pasp/ (free online)
What it covers: Physics-based sound synthesis: waveguide models, resonators, modal synthesis. Not directly relevant to analysis, but useful for understanding the physical origins of the signals being analyzed (string vibration modes, drum membrane modes, etc.).

**Smith, J.O. (2007). Introduction to Digital Filters with Audio Applications. CCRMA, Stanford.**
URL: https://ccrma.stanford.edu/~jos/filters/ (free online)
What it covers: FIR/IIR filter theory and design with audio examples. Transfer functions, pole-zero analysis, biquad implementation, filter cascades.
When to read: When designing any filter in the analysis pipeline. Free and excellent.

**Lazzarini, V. (2017). Computer Music Instruments: Foundations, Design and Development. Springer.**
ISBN: `978-3-319-63559-0`
What it covers: Audio programming patterns, real-time scheduling, plugin architectures. Bridges DSP theory and software engineering.
When to read: When designing the architecture of the analysis engine.

---

## 3. GitHub Repositories

### 3.1 Audio Analysis Libraries

**aubio/aubio** -- https://github.com/aubio/aubio
C library for audio labeling: onset detection, pitch tracking (YIN, YINFFT), beat tracking, MFCC, mel filterbank. Lightweight, no heavy dependencies, MIT-like license (GPL-3.0). Core library evaluated in [LIB_aubio.md](LIB_aubio.md). Python bindings available.

**MTG/essentia** -- https://github.com/MTG/essentia
C++ library from the Music Technology Group (UPF Barcelona). 200+ algorithms covering spectral, tonal, rhythmic, and statistical audio descriptors. Standard and streaming processing modes. AGPL-3.0 (commercial license available). Detailed evaluation in [LIB_essentia.md](LIB_essentia.md).

**librosa/librosa** -- https://github.com/librosa/librosa
Python library for audio analysis. Implements most MIR features: STFT, CQT, mel spectrograms, MFCCs, chroma, onset detection, beat tracking, harmonic-percussive separation. ISC license. Not suitable for production real-time C++ but invaluable for prototyping and reference validation.

**bmcfee/mir_eval** -- https://github.com/craffel/mir_eval
Python library implementing standard MIR evaluation metrics. Use for quantitative validation of onset detection (F-measure), beat tracking, pitch estimation, and structural segmentation. MIT license.

**madmom** -- https://github.com/CPJKU/madmom
Python library with state-of-the-art deep-learning-based onset detection, beat tracking, downbeat detection, and tempo estimation. Uses pre-trained neural networks. Not real-time on CPU, but useful for generating ground-truth annotations. BSD license.

### 3.2 Audio I/O Libraries

**mackron/miniaudio** -- https://github.com/mackron/miniaudio
Single-header C library for audio playback and capture. Supports WASAPI, DirectSound, WinMM (Windows), CoreAudio (macOS), ALSA, PulseAudio, JACK (Linux), AAudio, OpenSL|ES (Android). Built-in resampler, format conversion, and loopback capture on Windows. Public domain (Unlicense OR MIT-0). Recommended for this project; see [ARCH_audio_io.md](ARCH_audio_io.md).

**thestk/rtaudio** -- https://github.com/thestk/rtaudio
C++ class providing a common API across audio backends (ASIO, WASAPI, CoreAudio, ALSA, JACK, PulseAudio). Simpler API than PortAudio, but fewer features than miniaudio. MIT-like license.

**PortAudio/portaudio** -- https://github.com/PortAudio/portaudio
C library for cross-platform audio I/O. Mature, widely used (Audacity uses it). Supports ASIO on Windows. MIT license. Heavier than miniaudio but battle-tested.

**juce-framework/JUCE** -- https://github.com/juce-framework/JUCE
Full framework for audio applications and plugins. Includes audio I/O, GUI toolkit, DSP modules, plugin hosting, and plugin formats (VST3, AU, AAX). Dual licensed: GPLv3 or commercial. Evaluated in [LIB_juce.md](LIB_juce.md). Overkill for pure analysis, but if the visualization is a plugin or needs a GUI, JUCE is the standard.

**libsoundio/libsoundio** -- https://github.com/andrewrk/libsoundio
C library for real-time audio I/O. Clean API, supports JACK, PulseAudio, ALSA, CoreAudio, WASAPI. MIT license. Less actively maintained than miniaudio.

### 3.3 FFT Libraries

**FFTW/fftw3** -- https://www.fftw.org/ (https://github.com/FFTW/fftw3)
"Fastest Fourier Transform in the West." Adaptive algorithm selection via planning. Supports arbitrary sizes, real/complex transforms, multidimensional. GPL-2.0 (commercial license available). The benchmark against which all other FFT libraries are measured. See [LIB_fft_comparison.md](LIB_fft_comparison.md).

**mborgerding/kissfft** -- https://github.com/mborgerding/kissfft
"Keep It Simple, Stupid FFT." Pure C, minimal code, BSD license. No SIMD optimization, but trivially embeddable. Good enough for prototyping; ~2-3x slower than FFTW for typical sizes.

**marton78/pffft** -- https://github.com/marton78/pffft
"Pretty Fast FFT." SIMD-optimized (SSE, NEON), BSD license, single-file, power-of-2 sizes only. Performance within 2x of FFTW for power-of-2 sizes. Excellent balance of speed and simplicity. See [LIB_fft_comparison.md](LIB_fft_comparison.md).

**kfrlib/kfr** -- https://github.com/kfrlib/kfr
C++ DSP framework with SIMD-optimized FFT, FIR/IIR filters, sample rate conversion, and window functions. Competitive with FFTW for power-of-2 sizes. GPL-2.0 or commercial.

**vDSP (Apple Accelerate)** -- https://developer.apple.com/documentation/accelerate/vdsp
Apple's SIMD-optimized FFT. Part of the Accelerate framework (no separate dependency). Matches or exceeds FFTW on Apple Silicon. The recommended FFT backend for macOS-only builds. See [LIB_essentia.md](LIB_essentia.md) for Essentia integration with Accelerate.

### 3.4 Creative Coding Frameworks

**openframeworks/openFrameworks** -- https://github.com/openframeworks/openFrameworks
C++ toolkit for creative coding. Includes `ofSoundStream` for audio I/O, `ofxFft` addon for FFT, and OpenGL rendering. MIT license. Popular for audio-visual installations.

**cinder/Cinder** -- https://github.com/cinder/Cinder
C++ creative coding framework. Robust audio I/O (`ci::audio` namespace), built-in FFT, and OpenGL rendering. BSD license. More "engineered" than openFrameworks.

**nannou-org/nannou** -- https://github.com/nannou-org/nannou
Rust creative coding framework. Integrates with `cpal` for audio and `wgpu` for GPU rendering. MIT license. Relevant if the visualization engine is built in Rust.

### 3.5 Rust Audio Libraries

**RustAudio/cpal** -- https://github.com/RustAudio/cpal
Cross-platform audio I/O in Rust. Supports WASAPI, CoreAudio, ALSA, JACK. Apache-2.0. The Rust equivalent of miniaudio/PortAudio.

**ejmahler/RustFFT** -- https://github.com/ejmahler/RustFFT
FFT library in pure Rust. SIMD-optimized, competitive with FFTW for many sizes. Supports arbitrary (non-power-of-2) sizes. Apache-2.0 / MIT.

**SamiPerttu/fundsp** -- https://github.com/SamiPerttu/fundsp
Audio DSP library in Rust with a functional, composable API. Filters, oscillators, effects, analysis nodes. MIT / Apache-2.0.

**RustAudio/dasp** -- https://github.com/RustAudio/dasp
Digital audio signal processing in Rust. Sample types, frame types, ring buffers, interpolation, sample rate conversion. Apache-2.0 / MIT.

**nicholasbishop/pyin-rs** -- https://github.com/nicholasbishop/pyin-rs
Rust implementation of the pYIN pitch detection algorithm. Useful if building the analysis engine in Rust.

### 3.6 Other Notable Repositories

**superpoweredSDK/Low-Latency-Android-iOS-Linux-Windows-tvOS-macOS-Interactive-Audio-Platform** -- https://github.com/nicholasgasior/superpowered
(Official: https://superpowered.com/) Commercial SDK with FFT, time-stretching, and audio effects optimized for mobile. Free for evaluation.

**musikinformatik/SuperCollider** -- https://github.com/supercollider/supercollider
Real-time audio synthesis and algorithmic composition platform. Includes extensive analysis UGens (Onsets, BeatTrack, Pitch, MFCC, etc.). GPL-3.0. Useful for rapid prototyping of analysis chains.

**facebookresearch/demucs** -- https://github.com/facebookresearch/demucs
State-of-the-art music source separation (vocals, drums, bass, other). Not real-time, but could be used for offline pre-processing to generate separated stems for independent visualization.

**spotify/pedalboard** -- https://github.com/spotify/pedalboard
Python library for audio effects and I/O (wraps JUCE). Useful for prototyping audio processing chains. GPL-3.0.

**surge-synthesizer/surge** -- https://github.com/surge-synthesizer/surge
Open-source synthesizer with sophisticated DSP. Its filter and oscillator code is well-documented and can serve as reference implementations. GPL-3.0.

---

## 4. Tutorials and Online Courses

### 4.1 Structured Courses

**"Audio Signal Processing for Music Applications" (Coursera)**
Instructors: Xavier Serra, Julius O. Smith III, Jordi Bonada (UPF / Stanford)
URL: https://www.coursera.org/learn/audio-signal-processing
Covers spectral analysis (STFT, sinusoidal model), harmonic model, sound transformations, and music description. Hands-on Python programming with SMS-tools. The single best course for the intersection of DSP and MIR. Free to audit.

**"Music Information Retrieval" (Coursera / Georgia Tech)**
Instructor: Alexander Lerch
URL: https://www.coursera.org/learn/music-information-retrieval (or textbook: An Introduction to Audio Content Analysis)
Covers audio features, classification, tempo, beat tracking, pitch estimation, and evaluation. Complements the Serra/Smith course with more focus on MIR tasks.

**"Digital Signal Processing" (Coursera / EPFL)**
Instructors: Paolo Prandoni, Martin Vetterli
URL: https://www.coursera.org/learn/dsp
Rigorous DSP fundamentals: sampling, Fourier transforms, filter design, interpolation. Not audio-specific, but builds the mathematical foundation.

### 4.2 Free Online Books (Julius O. Smith III, Stanford CCRMA)

All available at https://ccrma.stanford.edu/~jos/

- **Mathematics of the DFT**: https://ccrma.stanford.edu/~jos/mdft/ -- FFT theory with audio focus
- **Introduction to Digital Filters**: https://ccrma.stanford.edu/~jos/filters/ -- filter design for audio
- **Physical Audio Signal Processing**: https://ccrma.stanford.edu/~jos/pasp/ -- physics-based sound modeling
- **Spectral Audio Signal Processing**: https://ccrma.stanford.edu/~jos/sasp/ -- advanced spectral methods (STFT, analysis-synthesis, spectral envelopes)

These four books collectively form the most comprehensive free DSP/audio resource available. Read the first two for this project; the others are reference material.

### 4.3 YouTube Channels and Video Resources

**The Audio Programmer** -- https://www.youtube.com/@TheAudioProgrammer
C++ audio development tutorials. Covers JUCE, audio plugins, DSP fundamentals. Active Discord community (see Section 5). Practical, implementation-focused content.

**Valerio Velardo -- The Sound of AI** -- https://www.youtube.com/@ValerioVelardo
Python-based audio/music AI tutorials. Mel spectrograms, MFCCs, deep learning for audio. Good for building intuition about audio features.

**3Blue1Brown -- "But what is the Fourier Transform?"** -- https://www.youtube.com/watch?v=spUNpyF58BY
Best visual explanation of the Fourier transform. Watch before reading any mathematical treatment.

**Gonkee -- "How Do Computers Generate Audio?"** -- https://www.youtube.com/@Gonkee
Accessible explanations of audio synthesis and processing concepts.

### 4.4 Websites and Archives

**music-dsp.org** -- http://www.music-dsp.org/
Archive of DSP code snippets, formulas, and algorithms contributed by the music-dsp mailing list community. Categorized by topic (filters, effects, analysis, synthesis). Contains practical implementations of biquad filters, envelope followers, pitch detectors, and more. Some entries are decades old but the math hasn't changed.

**The Scientist and Engineer's Guide to Digital Signal Processing** -- https://www.dspguide.com/
Free online DSP textbook by Steven W. Smith (not the same as Julius O. Smith). More engineering-focused, covers ADC/DAC, filter design, FFT, image processing. Good supplementary reading.

**musicinformationretrieval.com** -- https://musicinformationretrieval.com/
Free MIR tutorial notebooks by Steve Tjoa. Covers audio features, genre classification, onset detection, chord recognition. Jupyter notebook format with interactive Python code.

**Seeing Circles, Sines, and Signals** -- https://jackschaedler.github.io/circles-sines-signals/
Interactive visual introduction to DSP. Covers sampling, aliasing, DFT, convolution, and filtering with animated diagrams. Excellent for building intuition.

---

## 5. Communities

### 5.1 Mailing Lists

**music-dsp mailing list**
URL: https://lists.columbia.edu/mailman/listinfo/music-dsp
The oldest and most authoritative community for audio DSP. Active since the mid-1990s. Archives contain decades of expert discussion on filters, FFTs, pitch detection, effects algorithms, and real-time optimization. Many seminal audio algorithms were first discussed here before being published.

**CCRMA mailing lists (Stanford)**
URL: https://ccrma.stanford.edu/
Various lists for different topics (Faust, ChucK, etc.). Lower volume but high expertise density.

### 5.2 Forums

**KVR Audio Developer Forum** -- https://www.kvraudio.com/forum/viewforum.php?f=33
Forum for audio plugin and application developers. Discussions on DSP algorithms, plugin architectures, licensing, and platform-specific issues. Active and knowledgeable community.

**JUCE Forum** -- https://forum.juce.com/
Official forum for JUCE users and developers. Excellent for JUCE-specific questions, but also covers general audio DSP topics. Active development team responds.

**DSP StackExchange** -- https://dsp.stackexchange.com/
Q&A site for DSP. Tagged questions on audio processing, filter design, spectral analysis. Good for specific technical questions with well-defined answers.

### 5.3 Reddit

**r/DSP** -- https://www.reddit.com/r/DSP/
General DSP discussion. Mix of theory questions, implementation help, and career advice. Moderate activity.

**r/audiodev** -- https://www.reddit.com/r/audiodev/
Audio development discussion. Plugin development, real-time audio, engine architecture. Lower volume but focused.

**r/musicprogramming** -- https://www.reddit.com/r/musicprogramming/
Broader than r/audiodev: includes algorithmic composition, live coding, interactive music. Relevant for creative aspects of visualization.

### 5.4 Discord

**The Audio Programmer Discord** -- https://www.theaudioprogrammer.com/discord
Active community (~20k members) for audio programming. Channels for JUCE, Rust audio, DSP theory, plugin development. Good for real-time help and code review.

**Rust Audio Discord** -- https://discord.gg/rust-audio (linked from https://rust.audio/)
Community for audio programming in Rust. Discussions on cpal, fundsp, and Rust-specific audio patterns.

### 5.5 Conferences and Workshops

**ISMIR** (International Society for Music Information Retrieval) -- https://ismir.net/
Annual conference. The primary venue for MIR research. Proceedings are open-access. All cited MIR papers from this document are from ISMIR or closely related venues.

**DAFx** (International Conference on Digital Audio Effects) -- https://dafx.de/
Annual conference. Focuses on audio effects, real-time processing, and synthesis. More engineering-focused than ISMIR.

**AES Convention** (Audio Engineering Society) -- https://www.aes.org/
Professional society for audio engineering. Conventions include papers on measurement, loudness, and processing standards.

**ADC** (Audio Developer Conference) -- https://audio.dev/
Annual conference for audio software developers. Presentations on real-time architectures, plugin development, and DSP optimization. Recordings available online.

---

## 6. Datasets for Testing and Validation

### 6.1 Beat Tracking and Tempo

**Ballroom Dance Dataset**
URL: http://mtg.upf.edu/ismir2004/contest/tempoContest/node5.html
698 excerpts of ballroom dance music (30 seconds each) with BPM annotations. Genres: ChaChaCha, Jive, Quickstep, Rumba, Samba, Tango, Viennese Waltz, Waltz.
Use for: Validating tempo estimation accuracy. The restricted genre set (all have clear beats) makes this a good starting benchmark before testing on more challenging material.

**GTZAN Rhythm Dataset (subset)**
Part of the GTZAN collection (see below). Beat and tempo annotations available through MIREX submissions.
Use for: Genre-diverse tempo evaluation.

**SMC Dataset (Sound and Music Computing)**
URL: https://zenodo.org/record/4297507
288 excerpts with hand-annotated beat positions. Includes challenging material (solo instruments, rubato, complex meters).
Use for: Stress-testing beat trackers. If your beat tracker works on SMC, it will work on typical music.

### 6.2 Genre Classification and General Features

**GTZAN Genre Dataset**
URL: http://marsyas.info/downloads/datasets.html
1000 audio clips (30 seconds each), 10 genres (blues, classical, country, disco, hiphop, jazz, metal, pop, reggae, rock), 100 per genre. 22050 Hz mono WAV.
Use for: Genre classification benchmarks, general feature extraction validation. Despite known label noise and repetition issues (Sturm, 2013), it remains the most widely used genre dataset. Use it for comparative testing, not for training.

**FMA (Free Music Archive) Dataset**
URL: https://github.com/mdeff/fma
106,574 tracks with genre labels, Creative Commons licensed. Four subsets: Small (8k tracks, 30s, 8 genres), Medium (25k tracks, 30s, 16 genres), Large (106k tracks, 30s, 161 genres), Full (106k full tracks).
Use for: Large-scale feature extraction testing. The "Small" subset is manageable for development; "Medium" covers enough diversity for thorough testing.

### 6.3 Onset Detection

**MIREX Onset Detection Dataset**
URL: https://www.music-ir.org/mirex/wiki/2019:Audio_Onset_Detection
Annotated datasets used in the MIREX (Music Information Retrieval Evaluation eXchange) onset detection task. Multiple subsets covering different instrument types and complexities.
Use for: Standard onset detection evaluation using F-measure with 50ms tolerance window. Use `mir_eval.onset.f_measure()` for computation.

**Böck et al. Onset Dataset**
Used in the madmom library evaluation. Contains ~300 annotated excerpts covering orchestral, solo, and mixed instrumental music.
Use for: Validating onset detection on difficult polyphonic material.

### 6.4 Pitch and Melody

**MIR-1K Dataset**
URL: https://sites.google.com/site/unaboraboraithz/mir-1k
1000 karaoke clips with pitch annotations. Separate vocal and accompaniment tracks.
Use for: Pitch estimation evaluation, especially melody extraction from polyphonic audio. Use `mir_eval.melody` metrics.

**MDB-stem-synth (MedleyDB Pitch Tracking Subset)**
URL: https://zenodo.org/record/1481172
Synthesized stems from MedleyDB with exact pitch annotations (since they're synthesized, the pitch is known precisely).
Use for: Ground-truth pitch validation without annotation uncertainty.

### 6.5 General Music Databases

**RWC Music Database**
URL: https://staff.aist.go.jp/m.goto/RWC-MDB/
Royalty-free, annotated music database with multiple sub-databases: Popular Music (100 songs), Royalty-Free Music (15 songs), Classical Music (50 pieces), Jazz (50 pieces), Genre (100 pieces). Each track has structural, beat, melody, and chord annotations.
Use for: The most comprehensively annotated music database available. Ideal for end-to-end validation of the complete feature extraction pipeline.

**MusicBrainz** -- https://musicbrainz.org/
Open music encyclopedia with metadata for millions of recordings. Not an audio dataset, but provides ground-truth metadata (key, tempo, genre) for tracks you already have.

**AcousticBrainz** -- https://acousticbrainz.org/ (archived 2022, data still available)
Community-contributed acoustic features (lowlevel, rhythm, tonal, highlevel) for millions of tracks, computed using Essentia. Not raw audio, but pre-computed features.
Use for: Cross-validating your feature extractors against Essentia's reference output on the same tracks.

### 6.6 Structural Segmentation

**SALAMI (Structural Analysis of Large Amounts of Music Information)**
URL: https://ddmal.music.mcgill.ca/research/SALAMI/
Structural annotations (verse, chorus, bridge, etc.) for ~1,400 tracks.
Use for: Evaluating structural boundary detection algorithms from [FEATURES_structural.md](FEATURES_structural.md).

### 6.7 Using Datasets for Validation

The recommended validation workflow:

1. **Unit testing**: Generate synthetic signals (pure tones, click trains, swept sines) with known properties. Verify that each feature extractor produces the expected output. No dataset needed.

2. **Reference validation**: Run librosa and/or Essentia (Python) on a small set of real audio files. Compare C++ feature extractor output against these reference values. Use tolerance thresholds appropriate to each feature (e.g., +/-1 BPM for tempo, +/-50 cents for pitch, +/-5ms for onsets).

3. **Benchmark evaluation**: Use MIREX-standard datasets (Ballroom for tempo, MIREX onset set for onsets, MIR-1K for pitch) with `mir_eval` metrics. Report F-measure for onsets/beats, accuracy for tempo, raw pitch accuracy for pitch.

4. **Stress testing**: Test on edge cases -- silence, noise, very slow/fast tempo, very low/high pitch, key changes, time signature changes, fade-ins/outs, live recordings with audience noise.

See [IMPL_testing_validation.md](IMPL_testing_validation.md) for the complete validation framework.

---

## 7. Online Tools and Visualizers

### 7.1 Analysis Tools

**Sonic Visualiser** -- https://www.sonicvisualiser.org/
Open-source (GPL) audio analysis and annotation tool from the Centre for Digital Music (Queen Mary, University of London). Supports spectrogram, waveform, and note display. Extensible via Vamp plugins -- hundreds of analysis plugins available (onset detection, pitch tracking, chord estimation, beat tracking). The pYIN Vamp plugin is the reference implementation for pitch tracking validation.
Use for: Visual inspection of audio features, generating ground-truth annotations, validating feature extractor output.

**Audacity** -- https://www.audacityteam.org/
Open-source audio editor with built-in spectrum analyzer (Plot Spectrum), spectrogram view, and basic measurement tools. Supports Nyquist plugins for custom analysis.
Use for: Quick visual inspection, format conversion, basic spectral analysis. Not as capable as Sonic Visualiser for research-grade analysis.

**Friture** -- https://friture.org/
Open-source (GPL) real-time audio analysis tool. Displays spectrum, spectrogram, octave spectrum, and delay estimation in real-time. Written in Python with PyQt.
Use for: Real-time visualization of audio input for debugging and calibration. Useful for verifying that your analysis engine's frequency-domain output matches an independent tool.

**Spek** -- https://spek.cc/
Acoustic spectrum analyser. Displays a spectrogram of an entire audio file. Useful for quick visual inspection of frequency content, clipping, and encoding artifacts.

### 7.2 Web-Based Tools

**Chrome Music Lab: Spectrogram** -- https://musiclab.chromeexperiments.com/Spectrogram/
Browser-based real-time spectrogram using Web Audio API. Captures microphone input and displays a scrolling spectrogram.
Use for: Quick demonstrations and intuition-building. Not suitable for precise analysis.

**FFT Explorer** -- https://github.com/nicholasgasior/fft-explorer (various implementations)
Interactive web tools for exploring FFT parameters (window size, overlap, window function). Useful for understanding the time-frequency tradeoff.

### 7.3 Profiling and Debugging Tools

**Tracy Profiler** -- https://github.com/wolfpld/tracy
Real-time C++ profiler with nanosecond resolution. Supports CPU zones, GPU zones, memory allocation tracking, and lock contention analysis. Essential for profiling the real-time analysis pipeline. Referenced in [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md).

**Instruments (macOS)** -- Built into Xcode
Apple's profiling suite. Audio-specific templates for tracking audio callback timing, buffer underruns, and thread scheduling. The "Audio System Trace" template is particularly relevant.

**RenderDoc** -- https://github.com/baldurk/renderdoc
GPU debugger and frame analyzer. Useful if the visualization engine uses GPU compute shaders for FFT or feature extraction. Referenced in [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md).

---

## 8. Standards and Specifications

### 8.1 Loudness Measurement

**ITU-R BS.1770-5 (2023): "Algorithms to Measure Audio Programme Loudness and True-Peak Audio Level"**
URL: https://www.itu.int/rec/R-REC-BS.1770
Defines: K-weighting filter (pre-filter + RLB-weighting), gated loudness measurement (LUFS/LKFS), loudness range (LRA), true-peak measurement using 4x oversampling.
Relevance: The K-weighting filter coefficients are defined here. LUFS measurement is the standard for loudness normalization of incoming audio. Referenced in [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md).

**EBU R128 (2020): "Loudness Normalisation and Permitted Maximum Level of Audio Signals"**
URL: https://tech.ebu.ch/docs/r/r128.pdf
Defines: Target loudness (-23 LUFS), permitted true-peak level (-1 dBTP), loudness range (LRA) measurement.
Relevance: Practical application of ITU-R BS.1770. If the analysis engine normalizes input loudness, R128 defines the target level and measurement procedure. The LRA metric (difference between 10th and 95th percentile of short-term loudness) is a useful dynamic range descriptor.

**EBU Tech 3341 (2016): "Loudness Metering: 'EBU Mode' Metering to Supplement EBU R128"**
URL: https://tech.ebu.ch/docs/tech/tech3341.pdf
Defines: Momentary loudness (400ms gate), short-term loudness (3s gate), integrated loudness (full program), and the "EBU mode" meter ballistics.
Relevance: Specifies the exact integration times for loudness measurement. The 400ms momentary window is a useful time constant for visualization-driving loudness.

### 8.2 Equal-Loudness and Psychoacoustic Standards

**ISO 226:2003: "Acoustics -- Normal Equal-Loudness-Level Contours"**
URL: https://www.iso.org/standard/34222.html
Defines: Equal-loudness contours (updated from the original Fletcher-Munson curves). Maps SPL to perceived loudness (phon) as a function of frequency.
Relevance: The basis for A-weighting and any perceptual frequency weighting. If visualizations should reflect perceived loudness rather than physical amplitude, these contours define the correction. Referenced in [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md).

**ISO 532-1:2017: "Acoustics -- Methods for Calculating Loudness -- Part 1: Zwicker Method"**
URL: https://www.iso.org/standard/63077.html
Defines: The Zwicker loudness model for stationary sounds. Computes specific loudness as a function of critical band rate (Bark), then integrates over Bark to get total loudness in sone.
Relevance: The standard computational loudness model. More accurate than simple dB/dBA measurement for varying spectral content. Referenced in [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md).

**ISO 532-2:2017: "Acoustics -- Methods for Calculating Loudness -- Part 2: Moore-Glasberg Method"**
URL: https://www.iso.org/standard/63078.html
Defines: Alternative loudness model using ERB (equivalent rectangular bandwidth) scale instead of Bark. More modern than Zwicker but less widely implemented in audio software.
Relevance: Alternative to Zwicker for loudness computation. The ERB scale is increasingly preferred in computational auditory models.

### 8.3 Audio Measurement

**AES17-2020: "AES Standard for Digital Audio -- Measurement"**
URL: https://www.aes.org/publications/standards/search.cfm?docID=21
Defines: Standard measurement procedures for digital audio equipment: frequency response, THD+N, dynamic range, interchannel phase, jitter.
Relevance: Defines the measurement methodology for validating audio I/O quality. If the analysis engine measures THD or SNR, AES17 defines how.

**AES-6id-2006: "AES Information Document for Digital Audio -- Personal Computer Audio Quality Measurements"**
URL: https://www.aes.org/publications/standards/
Practical guide for measuring audio quality on PCs. Covers soundcard loopback testing, driver latency measurement, and buffer size optimization.
Relevance: Useful when characterizing audio interface latency and quality for the capture pipeline described in [ARCH_audio_io.md](ARCH_audio_io.md).

### 8.4 Audio Coding and Format Standards

**IEC 61672-1:2013: "Electroacoustics -- Sound Level Meters -- Part 1: Specifications"**
Defines: A-weighting, C-weighting, and Z-weighting filter specifications.
Relevance: If implementing A-weighted level measurement, this standard defines the exact filter coefficients and tolerances.

**MIDI 2.0 Specification** -- https://www.midi.org/specifications/midi-2-0-specifications
Defines: Universal MIDI Packet format, MIDI-CI (capability inquiry), property exchange.
Relevance: If the visualization engine accepts MIDI input (from a DAW or controller), MIDI 2.0 provides higher resolution (32-bit velocity, per-note controllers).

---

## 9. Cross-Reference Index

Quick lookup: which resources are referenced in which documents.

| Resource | Referenced In |
|----------|--------------|
| Bello et al. (2005) -- Onset Detection Tutorial | FEATURES_rhythm_tempo, FEATURES_transients_texture |
| de Cheveigné & Kawahara (2002) -- YIN | FEATURES_pitch_harmonic, LIB_aubio, LIB_essentia |
| Mauch & Dixon (2014) -- pYIN | FEATURES_pitch_harmonic |
| Dixon (2007) -- BeatRoot | FEATURES_rhythm_tempo |
| Ellis (2007) -- Beat Tracking by DP | FEATURES_rhythm_tempo |
| Scheirer (1998) -- Tempo and Beat | FEATURES_rhythm_tempo |
| Fitzgerald (2010) -- HPSS | FEATURES_transients_texture |
| Brown (1991) -- CQT | FEATURES_spectral, FEATURES_pitch_harmonic |
| Zwicker & Fastl (2007) -- Psychoacoustics | FEATURES_psychoacoustic, FEATURES_transients_texture, FEATURES_amplitude_dynamics |
| Krumhansl (1990) -- Key Profiles | FEATURES_pitch_harmonic |
| Müller (2015) -- FMP | All FEATURES documents |
| Foote (2000) -- Novelty Function | FEATURES_structural |
| Salamon & Gómez (2012) -- MELODIA | FEATURES_pitch_harmonic, LIB_essentia |
| ITU-R BS.1770 -- Loudness | FEATURES_amplitude_dynamics |
| Bogdanov et al. (2013) -- Essentia | LIB_essentia |
| Brossier (2006) -- aubio thesis | LIB_aubio |
| miniaudio | ARCH_audio_io |
| FFTW / pffft / KissFFT | LIB_fft_comparison |
| Tracy Profiler | ARCH_realtime_constraints |
| mir_eval | IMPL_testing_validation |
| Sonic Visualiser | IMPL_testing_validation |

---

## 10. Recommended Reading Order

For a developer building a real-time audio analysis engine for music visualization:

**Phase 1 -- Foundations (Week 1-2)**
1. 3Blue1Brown Fourier Transform video (30 min)
2. Smith, Mathematics of the DFT, Chapters 1-7 (free online)
3. Müller, Fundamentals of Music Processing, Chapters 1-3 (audio representations, Fourier, STFT)
4. Bello et al. (2005) -- onset detection tutorial

**Phase 2 -- Core Features (Week 2-4)**
5. Müller, FMP, Chapters 4-6 (tempo, beat, structure)
6. de Cheveigné & Kawahara (2002) -- YIN
7. Davis & Mermelstein (1980) -- MFCCs
8. Brown (1991) -- CQT
9. Fitzgerald (2010) -- HPSS
10. Coursera: Audio Signal Processing for Music Applications (ongoing)

**Phase 3 -- Implementation (Week 4-6)**
11. Pirkle, Designing Audio Effect Plugins in C++, filter chapters
12. Smith, Introduction to Digital Filters (free online), biquad sections
13. Zölzer, DAFX, relevant chapters
14. ARCH_pipeline.md and ARCH_realtime_constraints.md from this library

**Phase 4 -- Perceptual Refinement (Week 6-8)**
15. Moore, Introduction to Psychology of Hearing, Chapters 1-4
16. Zwicker & Fastl, Psychoacoustics, loudness and roughness chapters
17. Krumhansl (1990), key-finding chapter
18. Bregman, Auditory Scene Analysis, Part I (sequential grouping)

**Phase 5 -- Advanced and Ongoing**
19. Remaining Müller FMP chapters
20. Deep learning papers (Böck, Kong) for offline pre-analysis
21. Conference proceedings (ISMIR, DAFx) for state-of-the-art methods

---

*This document is the master reference index for the real-time audio analysis research library. All other documents in the library reference specific resources listed here. Updated as new resources are discovered or evaluated.*
