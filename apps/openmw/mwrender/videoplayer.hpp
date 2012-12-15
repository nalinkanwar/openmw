#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <OgreRoot.h>
#include <OgreHardwarePixelBuffer.h>

#include <boost/thread.hpp>


#define __STDC_CONSTANT_MACROS
#include <stdint.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <cstdio>
#include <cmath>

#include "../mwbase/soundmanager.hpp"


namespace MWRender
{
    struct VideoState;

    class VideoPlayer
    {
    public:
        VideoPlayer(Ogre::SceneManager* sceneMgr);
        ~VideoPlayer();

        void playVideo (const std::string& resourceName);

        void update();

        void close();

        bool isPlaying();

        void setResolution (int w, int h) { mWidth = w; mHeight = h; }


    private:
        VideoState* mState;

        Ogre::SceneManager* mSceneMgr;
        Ogre::MaterialPtr mVideoMaterial;
        Ogre::Rectangle2D* mRectangle;
        Ogre::Rectangle2D* mBackgroundRectangle;
        Ogre::SceneNode* mNode;
        Ogre::SceneNode* mBackgroundNode;

        int mWidth;
        int mHeight;
    };
}

#endif
