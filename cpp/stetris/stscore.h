#ifndef __STSCORE_H__
#define __STSCORE_H__

class CStScore
{
public:
    // Constructor
    CStScore();

    // Clear
    void Clear();

	// Add score
	void Add(unsigned int dwScore);
	// Set score
	void Set(unsigned int dwScore);
	// Get score
	unsigned int Get();

private:
	// Score
    unsigned int m_dwScore;
};

#endif
