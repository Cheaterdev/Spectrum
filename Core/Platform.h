
std::string file_open(const std::string& Name, const std::string& StartPath, const std::string& Extension);

class SystemInfo : public Singleton<SystemInfo>
{
    public:
        SystemInfo();
        ~SystemInfo();

        int GetCoreCount() const { return nCoreCount; }
    protected:
        int nCoreCount;
        int CpuFrequency;
};
