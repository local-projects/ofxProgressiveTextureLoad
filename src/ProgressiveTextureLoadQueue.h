//
//  ProgressiveTextureLoadQueue.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 23/09/14.
//
//

#ifndef BaseApp_ProgressiveTextureLoadQueue_h
#define BaseApp_ProgressiveTextureLoadQueue_h

#include "ofxProgressiveTextureLoad.h"

class ProgressiveTextureLoadQueue{

public:

	static inline ProgressiveTextureLoadQueue* instance(){
		if (!singleton){   // Only allow one instance of class to be generated.
			singleton = new ProgressiveTextureLoadQueue();
		}
		return singleton;
	}

	//its your responsibility to add listeners to the returned object (ret)
	//to get notified when the texture is fully/partially loaded with:
	//ofAddListener(ret->textureReady, this, &testApp::textureReady);
	//ofAddListener(ret->textureDrawable, this, &testApp::textureDrawable);
	ofxProgressiveTextureLoad* loadTexture(string path,
										   ofTexture* tex,
										   bool createMipMaps,
										   bool ARB,
										   int resizeQuality,
										   bool highPriority);

	void draw(int x, int y);
	string getStatsAsText();

	void update( ofEventArgs & args ); //dont call it, it happens automatically!

	//for each update() call (one frame), the addon will loop uploading texture regions
	//as scanlines, until we reach the target time per frame to be spent uploading texture data
	//how many scanlines per loop will determine the granularity of the time accuracy. Less scanlines
	//add more overhead, but should lead to more accurate stop times.
	void setScanlinesPerLoop(int numLines);

	//how much time do you want ofxProgressiveTextureLoad to spend uploading data to the gpu per frame
	void setTargetTimePerFrame(float ms);

	//how much of the queue can we dispatch per frame
	void setMaximumRequestsPerFrame(int max){maxRequestsPerFrame = max;};

	//how many textures to load at the same time! (threads)
	OF_DEPRECATED_MSG("Use setMaxThreads() instead.", void setNumberSimultaneousLoads(int numThreads) );
	void setMaxThreads(int numThreads);

	//in mipmap levels. used to tweak which mipmap to use; helps make mipmaps sharper or blurrier.
	//0 is neutral; negative is lower mipmaps (sharper), positive is higher mipmaps (blurrier)
	void setTexLodBias(float bias){texLodBias = bias;}


	//set debug print logging
	void setVerbose(bool v){verbose = v;}

	int getNumBusy();
	int getNumPendingTextures(){return pending.size() + current.size(); }

protected:

	ProgressiveTextureLoadQueue(); //use instance()!

	struct LoadRequest{
		int ID;
		string path;
		bool withMipMaps;
		ofxProgressiveTextureLoad * loader;
		LoadRequest(){
			loader = NULL;
		}
	};


	static ProgressiveTextureLoadQueue*		singleton;

	// queues //

	vector<LoadRequest> 					pending;
	vector<LoadRequest>						current;

	// params //

	int 				maxSimlutaneousThreads;
	int 				numLinesPerLoop; //we can increase that to reduce overhead
	float 				maxTimeTakenPerFrame; //ms to spend loading tex data on a single frame
	int					maxRequestsPerFrame;
	float				texLodBias;
	int					ids;
	bool				verbose;

	vector<ofxProgressiveTextureLoad*> leakedObjects;
	//those are objects whose thread is done, but the os is not ready for them to be deleted.
	//This seems to happens on some OSs (ios) where one every N threads get into this weird state
	//where they exited gracefully, but detach() failed and they can0t be joined() either.
	//you can tell bc after the thread is done, querying the thread object for its ID returns a
	//valid ID (get_id() != std::thread::id()).
	//For some reason if we try to delete them it triggers a SIGABRT, so we at least store them
	//here to keep track of them. as the list grows, there might be a point in which no moer threads
	//cant be spawned (unclear TODO needs testing)

};

#endif
