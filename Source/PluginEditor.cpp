#include "PluginEditor.h"
#include "PluginProcessor.h"

DigitalSynthesizerAudioProcessorEditor::DigitalSynthesizerAudioProcessorEditor(DigitalSynthesizerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    getLookAndFeel().setDefaultSansSerifTypefaceName(UI::Fonts::defaultFont.getTypefaceName());

    auto & state = p.getAPVTS().state;
    auto themes = UI::Colors::getAvailableThemeNames();
    int  defaultThemeID = themes.front().first;
    int  themeID = static_cast<int>(state.getProperty("themeID", defaultThemeID));
    UI::Colors::applyThemeByID(themeID);

    contentComponent = std::make_unique<juce::Component>();
    addAndMakeVisible(contentComponent.get());

    // Menu bar setup
    menuBar = std::make_unique<MenuBar>(audioProcessor);
    menuBar->updateTheme();
    menuBar->setOnThemeChanged([this]()
        {
            repaint();
            if (contentComponent)
            {
                contentComponent->repaint();

                for (auto* child : contentComponent->getChildren())
                {
                    child->repaint();

                    if (auto* osc = dynamic_cast<OscillatorComponent*>(child))
                        osc->updateTheme();
                    else if (auto* env = dynamic_cast<EnvelopeComponent*>(child))
                        env->updateTheme();
                    else if (auto* filter = dynamic_cast<FilterComponent*>(child))
                        filter->updateTheme();
                    else if (auto* lfo = dynamic_cast<LFOComponent*>(child))
                        lfo->updateTheme();
                }
            }
        });
    contentComponent->addAndMakeVisible(menuBar.get());

    // Create Oscillators
    oscillators.reserve(NUM_OF_OSCILLATORS);
    for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
    {
        juce::String name = p.getOscillator(i)->getLinkableName();
        auto osc = std::make_unique<OscillatorComponent>(p.getAPVTS(), p, name, i);
        contentComponent->addAndMakeVisible(*osc);
        oscillators.push_back(std::move(osc));
    }

    // Create Envelopes
    const auto& targets = p.getLinkableTargets();
    envelopes.reserve(NUM_OF_ENVELOPES);
    for (int i = 0; i < NUM_OF_ENVELOPES; ++i)
    {
        juce::String name = p.getEnvelope(i)->getName();
        auto env = std::make_unique<EnvelopeComponent>(p.getAPVTS(), p, name, i, targets);
        contentComponent->addAndMakeVisible(*env);
        envelopes.push_back(std::move(env));
    }

    // Create Filters
    filters.reserve(NUM_OF_FILTERS);
    for (int i = 0; i < NUM_OF_FILTERS; ++i)
    {
        juce::String name = p.getFilter(i)->getName();
        auto filter = std::make_unique<FilterComponent>(p.getAPVTS(), p, name, i, targets);
        contentComponent->addAndMakeVisible(*filter);
        filters.push_back(std::move(filter));
    }

    // Create LFOs
    lfos.reserve(NUM_OF_LFOS);
    for (int i = 0; i < NUM_OF_LFOS; ++i)
    {
        juce::String name = p.getLFO(i)->getName();
        auto lfo = std::make_unique<LFOComponent>(p.getAPVTS(), p, name, i);
        contentComponent->addAndMakeVisible(*lfo);
        lfos.push_back(std::move(lfo));
    }

    // Volume Meter
    contentComponent->addAndMakeVisible(volumeMeter);
    volumeMeter.setAudioProcessorReference(audioProcessor);

    layoutContentComponents();

    // Load previous size from APVTS state if available
    if (state.hasProperty("editorWidth") && state.hasProperty("editorHeight"))
    {
        int savedWidth = static_cast<int>(state.getProperty("editorWidth"));
        int savedHeight = static_cast<int>(state.getProperty("editorHeight"));
        setSize(savedWidth, savedHeight);
    }
    else
    {
        updateEditorSize();
    }

    setResizable(true, true);

    // Get original size
    const int totalWidth = getWidth();
    const int totalHeight = getHeight();

    // Get screen bounds
    const auto display = juce::Desktop::getInstance().getDisplays().getMainDisplay();
    const int screenW = display.userArea.getWidth();
    const int screenH = display.userArea.getHeight();

    // Resize bounds: min = 0.25x, max = capped at screen
    const int minW = totalWidth / 4;
    const int minH = totalHeight / 4;
    const int maxW = juce::jmin(screenW, totalWidth * 4);
    const int maxH = juce::jmin(screenH, totalHeight * 4);

    setResizeLimits(minW, minH, maxW, maxH);

    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio(static_cast<double>(totalWidth) / totalHeight);
}

DigitalSynthesizerAudioProcessorEditor::~DigitalSynthesizerAudioProcessorEditor()
{
    // Save window size to APVTS state
    auto& state = audioProcessor.getAPVTS().state;
    state.setProperty("editorWidth", getWidth(), nullptr);
    state.setProperty("editorHeight", getHeight(), nullptr);

    audioProcessor.clearAllKnobs();

    for (auto& lfo : lfos)
        lfo.reset();

    for (auto& filter : filters)
        filter.reset();

    for (auto& envelope : envelopes)
        envelope.reset();

    for (auto& osc : oscillators)
        osc.reset();

    volumeMeter.cleanup();

    if (menuBar)
        menuBar->setOnThemeChanged(nullptr);

    volumeMeter.removeAllChildren();
    menuBar.reset();

    if (contentComponent)
    {
        contentComponent->removeAllChildren(); // Detach all children from parent
        contentComponent.reset();
    }

    audioProcessor.clearLinkOwnerships();
}

void DigitalSynthesizerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(UI::Colors::MainBackground);
    g.setColour(UI::Colors::MainText);
}

void DigitalSynthesizerAudioProcessorEditor::resized()
{
    layoutContentComponents();

    const int menuHeight = menuBar->getHeight();

    const int oscWidth = OscillatorComponent::getTotalWidth();
    const int envWidth = EnvelopeComponent::getTotalWidth();
    const int filterWidth = FilterComponent::getTotalWidth();
    const int meterWidth = volumeMeter.getTotalWidth();
    const int oscHeight = OscillatorComponent::getTotalHeight();
    const int lfoHeight = LFOComponent::getTotalHeight();

    const int baseWidth = margin + oscWidth + margin + envWidth + margin + filterWidth + margin + meterWidth + margin;
    const int baseHeight = menuHeight + margin + (oscHeight + margin) * NUM_OF_OSCILLATORS + lfoHeight + margin;

    if (contentComponent)
        contentComponent->setBounds(0, 0, baseWidth, baseHeight);

    float scale = static_cast<float>(getWidth()) / baseWidth;
    contentComponent->setTransform(juce::AffineTransform::scale(scale));
}

void DigitalSynthesizerAudioProcessorEditor::updateEditorSize()
{
    const int menuHeight = menuBar->getHeight();

    const int oscWidth = OscillatorComponent::getTotalWidth();
    const int oscHeight = OscillatorComponent::getTotalHeight();

    const int envWidth = EnvelopeComponent::getTotalWidth();
    const int filterWidth = FilterComponent::getTotalWidth();
    const int meterWidth = volumeMeter.getTotalWidth();

    const int contentHeight = (oscHeight + margin) * NUM_OF_OSCILLATORS + LFOComponent::getTotalHeight() + margin;
    const int totalHeight = menuHeight + margin + contentHeight;

    const int totalWidth = margin + oscWidth + margin + envWidth + margin + filterWidth + margin + meterWidth + margin;

    setSize(totalWidth, totalHeight);
}

void DigitalSynthesizerAudioProcessorEditor::layoutContentComponents()
{
    const int menuHeight = menuBar->getHeight();
    const int oscWidth = OscillatorComponent::getTotalWidth();
    const int envWidth = EnvelopeComponent::getTotalWidth();
    const int filterWidth = FilterComponent::getTotalWidth();
    const int meterWidth = volumeMeter.getTotalWidth();
    const int oscHeight = OscillatorComponent::getTotalHeight();
    const int lfoWidth = LFOComponent::getTotalWidth();

    const int totalWidth = margin + oscWidth + margin + envWidth + margin + filterWidth + margin + meterWidth + margin;

    menuBar->setBounds(0, 0, totalWidth, menuHeight);

    int yPos = menuHeight + margin;

    for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
    {
        const int xOsc = margin;
        const int xEnv = xOsc + oscWidth + margin;
        const int xFilter = xEnv + envWidth + margin;

        if (i < oscillators.size())
            oscillators[i]->setBounds(xOsc, yPos, oscWidth, oscHeight);

        if (i < envelopes.size())
            envelopes[i]->setBounds(xEnv, yPos, envWidth, oscHeight);

        if (i < filters.size())
            filters[i]->setBounds(xFilter, yPos, filterWidth, oscHeight);

        yPos += oscHeight + margin;
    }

    int lfoX = margin;
    const int lfoY = yPos;

    for (int i = 0; i < NUM_OF_LFOS; ++i)
    {
        if (i < static_cast<int>(lfos.size()))
        {
            lfos[i]->setBounds(lfoX, lfoY, lfoWidth, LFOComponent::getTotalHeight());
            lfoX += lfoWidth + margin;
        }
    }

    const int meterX = totalWidth - meterWidth - margin;
    const int meterY = menuHeight + margin;
    const int meterHeight = (oscHeight + margin) * NUM_OF_OSCILLATORS + LFOComponent::getTotalHeight();
    volumeMeter.setBounds(meterX, meterY, meterWidth, meterHeight);
}
