#pragma once
namespace metatradenode {
	class MiningPublisher{
	public:
		virtual void PublishStart(int proveLevel) {};
		virtual void PublishFinished(int your_proof) {};
	};
};

