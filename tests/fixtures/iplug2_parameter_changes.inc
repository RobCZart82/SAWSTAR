// Extracted unchanged from iPlug2 d54f69050f517e43b941d88c2a170f0a840b9ee4.
// Copyright (C) the iPlug 2 developers. See third_party/licenses/iPlug2-LICENSE.txt.
void IPlugVST3ProcessorBase::ProcessParameterChanges(ProcessData& data, IPlugQueue<IMidiMsg>& fromProcessor)
{
  IParameterChanges* paramChanges = data.inputParameterChanges;
  
  if (paramChanges)
  {
    int32 numParamsChanged = paramChanges->getParameterCount();
    
    for (int32 i = 0; i < numParamsChanged; i++)
    {
      IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
      if (paramQueue)
      {
        int32 numPoints = paramQueue->getPointCount();
        int32 offsetSamples;
        double value;
        
        if (paramQueue->getPoint(numPoints - 1,  offsetSamples, value) == kResultTrue)
        {
          int idx = paramQueue->getParameterId();
          
          switch (idx)
          {
            case kBypassParam:
            {
              const bool bypassed = (value > 0.5);

              if (bypassed != GetBypassed())
                SetBypassed(bypassed);

              break;
            }
            default:
            {
              if (idx >= 0 && idx < mPlug.NParams())
              {
#ifdef PARAMS_MUTEX
                mPlug.mParams_mutex.Enter();
#endif
                mPlug.GetParam(idx)->SetNormalized(value);
              
                // In VST3 non distributed the same parameter value is also set via IPlugVST3Controller::setParamNormalized(ParamID tag, ParamValue value)
                mPlug.OnParamChange(idx, kHost, offsetSamples);
#ifdef PARAMS_MUTEX
                mPlug.mParams_mutex.Leave();
#endif
              }
              else if (idx >= kMIDICCParamStartIdx)
              {
                int index = idx - kMIDICCParamStartIdx;
                int channel = index / kCountCtrlNumber;
                int ctrlr = index % kCountCtrlNumber;

                IMidiMsg msg;

                if (ctrlr == kAfterTouch)
                  msg.MakeChannelATMsg((int) (value * 127.), offsetSamples, channel);
                else if (ctrlr == kPitchBend)
                  msg.MakePitchWheelMsg((value * 2.)-1., channel, offsetSamples);
                else
                  msg.MakeControlChangeMsg((IMidiMsg::EControlChangeMsg) ctrlr, value, channel, offsetSamples);

                fromProcessor.Push(msg);
                ProcessMidiMsg(msg);
              }
            }
              break;
          }
        }
      }
    }
  }
}
