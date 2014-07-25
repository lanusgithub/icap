#ifndef CROSS_SECTION_H__
#define CROSS_SECTION_H__

#include <string>
#include <vector>
#include <memory>

#include "../api.h"
#include "../util/parseable.h"

#include "types.h"


namespace xs
{
    class CrossSection : public Parseable
    {
    protected:
        xstype xsType;
    public:
        virtual bool setParameters(std::vector<std::string>::const_iterator firstPart, std::vector<std::string>::const_iterator end) = 0;

        virtual xstype getType() { return this->xsType; }
        virtual double getMaxDepth() = 0;

        virtual double computeArea(double depth) = 0;
        virtual double computeWettedPerimiter(double depth) = 0;
        virtual double computeTopWidth(double depth) = 0;
        virtual double computeDpDy(double depth) = 0;
        virtual double computeDaDy(double depth) = 0;
        virtual double computeDtDy(double depth) = 0;
    };

    class Factory
    {
    public:
        static std::shared_ptr<CrossSection> create(xstype xsType);
        static std::shared_ptr<CrossSection> create(const std::string& type);
        template<typename XSType> static std::shared_ptr<CrossSection> create();
    };
}


#endif//CROSS_SECTION_H__
