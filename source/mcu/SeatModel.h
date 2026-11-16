#pragma once
#include <qul/property.h>
#include <qul/singleton.h>

struct SeatModel : Qul::Singleton<SeatModel> {
    Qul::Property<int> selected;
    Qul::Property<int> heat;
    Qul::Property<int> cool;
    Qul::Property<bool> massage;
    Qul::Property<int> recline;
    Qul::Property<int> preview;
    Qul::Property<bool> saved;
    SeatModel();
    void select(int index);
    void setHeat(int level);
    void setCool(int level);
    void toggleMassage();
    void setRecline(int position);
    void save();
    void recall();
    void reset();
};
